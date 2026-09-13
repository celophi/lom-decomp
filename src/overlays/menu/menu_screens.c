#include "menu_internal.h"

/* Preserve the inline view available to the original input-handler callers. */
extern inline s32 menu_focus_active_content_item(void)
{
    MenuContentItem* content_items;
    s32 view_y;

    g_menu_hit_item_idx = menu_find_active_content_item();
    if (g_menu_hit_item_idx != (-1))
    {
        MenuNode* nodes = g_menu_nodes;
        u8 content_table_idx = (nodes + g_menu_scene_type)->idx_nav.s.self_idx;

        content_items = g_menu_content_table[content_table_idx];
        g_content_view_x = content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_X_MASK;
        view_y = content_items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET;
        g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
        g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
        g_content_view_y = view_y;
        return 1;
    }
    return 0;
}

/** @brief Return the party-sort order record for the active reorder screen. */
static inline MenuPartyOrder* menu_sort_order_record(void)
{
    u8* base = (u8*)g_pad_ctx;
    s32 off = (g_menu_scene_type == 0x1F) ? 0 : 0x250;

    return (MenuPartyOrder*)(base + off);
}


/* ----- Content Navigation and Input ----- */

/**
 * @brief Per-frame menu navigation, confirm/cancel and cursor-move input handler.
 * @param process_actions Non-zero to also process confirm/cancel and the four cursor-move buttons; 0 runs only node switching.
 * @return Unspecified; callers do not consume the return value.
 */
s32 menu_handle_input(s32 process_actions)
{
    MenuControllerActuatorState* actuator_state = MENU_CONTROLLER_ACTUATORS;
    const MenuContentItem* content_items;
    u32 content_type;
    MenuSlot* submenu_slot;
    s32 node_self_index;
    s32 sort_index_a, sort_index_b;
    u16 item_packed_x;
    u16 item_type;
    u8 action_group;
    u8 action_code;
    s32 work_index;
    MenuNode* selected_node;
    MenuNode* node_base;
    s32 node_nav_y;
    s32 scroll_extent;
    u8 item_flag;
    u32* inject_flags;
    u32 item_word;
    s32 text_variant;
    u8* name_table;
    u8* label_table;
    u8* description_table;
    u8* detail_table;
    MenuSlotRect rect;
    s8 text_buffer[0x40];
    s8* text_cursor;

    if ((u32)(g_menu_scene_type - 0x14) < 2 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1A || g_menu_scene_type == 0x1B)
    {
        if (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1))
        {
            menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
            if (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2))
            {
                if ((void*)g_menu_item_ptr != NULL)
                {
                    if (g_pad_input & PAD_BTN_L2)
                    {
                        menu_step_item_selection(-1);
                        if (g_script_repeat_last == 0)
                        {
                            g_script_repeat_last = g_menu_page_count - 1;
                        }
                        else
                        {
                            g_script_repeat_last -= 1;
                        }
                    }
                    else if (g_pad_input & PAD_BTN_R2)
                    {
                        menu_step_item_selection(1);
                        if (g_script_repeat_last == (g_menu_page_count - 1))
                        {
                            g_script_repeat_last = 0;
                        }
                        else
                        {
                            g_script_repeat_last += 1;
                        }
                    }
                }
            }
            else
            {
                if (g_menu_scene_type == 0x14 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x1A)
                {
                    g_menu_scene_type += 1;
                    g_menu_active_node = g_menu_scene_type;
                }
                else if (g_menu_scene_type == 0x15 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1B)
                {
                    g_menu_scene_type -= 1;
                    g_menu_active_node = g_menu_scene_type;
                }
                if (g_menu_cursor_enable == 0)
                {
                    menu_snap_view_to_cursor();
                }
                menu_set_active_node();
                menu_focus_active_content_item();
            }
        }
    }

    if (g_menu_scene_type < 0x11U && (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1)))
    {
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        switch (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx - 0x14)
        {
        case 0:
        case 3:
        case 6:
            g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx += 1;
            menu_focus_active_content_item();
            break;

        case 1:
        case 4:
        case 7:
            g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx -= 1;
            menu_focus_active_content_item();
            break;

        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
            break;

        case 2:
        case 5:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        default:
            while (1)
            {
                if (g_pad_input & PAD_BTN_L1)
                {
                    if (g_menu_scene_type == ((g_menu_scene_type / 3) * 3))
                    {
                        if (g_menu_scene_type >= 9)
                        {
                            g_menu_scene_type += 1;
                            g_menu_active_node = g_menu_scene_type;
                        }
                        else
                        {
                            g_menu_scene_type += 2;
                            g_menu_active_node = g_menu_scene_type;
                        }
                    }
                    else
                    {
                        g_menu_scene_type -= 1;
                        g_menu_active_node = g_menu_scene_type;
                    }
                }
                if (g_pad_input & PAD_BTN_R1)
                {
                    if (g_menu_scene_type < 9)
                    {
                        if ((g_menu_scene_type % 3) == 2)
                        {
                            g_menu_scene_type -= 2;
                            g_menu_active_node = g_menu_scene_type;
                        }
                        else
                        {
                            g_menu_scene_type += 1;
                            g_menu_active_node = g_menu_scene_type;
                        }
                    }
                    else
                    {
                        if ((g_menu_scene_type % 3) == 1)
                        {
                            g_menu_scene_type -= 1;
                            g_menu_active_node = g_menu_scene_type;
                        }
                        else
                        {
                            g_menu_scene_type += 1;
                            g_menu_active_node = g_menu_scene_type;
                        }
                    }
                }
                if ((g_pad_input & PAD_BTN_L2) && (g_menu_scene_type < 9))
                {
                    if ((g_menu_scene_type / 3) == 0)
                    {
                        g_menu_scene_type += 6;
                        g_menu_active_node = g_menu_scene_type;
                    }
                    else
                    {
                        g_menu_scene_type -= 3;
                        g_menu_active_node = g_menu_scene_type;
                    }
                    menu_set_active_node();
                }
                if ((g_pad_input & PAD_BTN_R2) && (g_menu_scene_type < 9))
                {
                    if ((g_menu_scene_type / 3) == 2)
                    {
                        g_menu_scene_type -= 6;
                        g_menu_active_node = g_menu_scene_type;
                    }
                    else
                    {
                        g_menu_scene_type += 3;
                        g_menu_active_node = g_menu_scene_type;
                    }
                    menu_set_active_node();
                }

                if (!(g_menu_nodes[(g_menu_scene_type / 3) * 3].u2.s.flags & MENU_NODE_FLAG_ACTIVE))
                {
                    continue;
                }
                if (g_menu_cursor_enable == 0)
                {
                    menu_snap_view_to_cursor();
                }
                if (menu_focus_active_content_item())
                {
                    break;
                }
            }

            menu_clear_slots();
            break;
        }
    }

    if (process_actions != 0)
    {
        content_items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];

        if ((g_menu_scene_type == 0x1F || g_menu_scene_type == 0x2B) && g_menu_hit_item_idx >= 0x11 && g_menu_hit_item_idx < 0x19)
        {
            if (g_pad_input & PAD_BTN_CIRCLE)
            {
                if (g_party_sort_marker.selected_idx != MENU_NONE)
                {
                    g_pad_input &= ~PAD_BTN_CIRCLE;
                    g_party_sort_marker.selected_idx = MENU_NONE;
                }
                else
                {
                    g_pad_input &= ~PAD_BTN_CIRCLE;
                    menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
                    menu_reset_content_view();
                }
            }
            if (g_pad_input & (PAD_BTN_CROSS | PAD_BTN_L3))
            {
                if (g_party_sort_marker.selected_idx == MENU_NONE)
                {
                    menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
                    g_party_sort_marker.x = (s8)(((MenuContentItemBits*)content_items)[g_menu_hit_item_idx].x - 2);
                    g_party_sort_marker.y = (s8)(content_items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET);
                    g_party_sort_marker.selected_idx = (u8)g_menu_hit_item_idx;
                }
                else if (g_party_sort_marker.selected_idx != g_menu_hit_item_idx)
                {
                    menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
                    for (sort_index_a = 0; sort_index_a < 8; sort_index_a++)
                    {
                        if (menu_sort_order_record()->order[sort_index_a] == (g_menu_hit_item_idx - 0x11))
                        {
                            break;
                        }
                    }
                    for (sort_index_b = 0; sort_index_b < 8; sort_index_b++)
                    {
                        if (menu_sort_order_record()->order[sort_index_b] == (g_party_sort_marker.selected_idx - 0x11))
                        {
                            break;
                        }
                    }
                    work_index = menu_sort_order_record()->order[sort_index_a];
                    {
                        u8* sort_base = (u8*)menu_sort_order_record();
                        u8* sort_dst = sort_base + sort_index_a + 0x638;
                        *sort_dst = menu_sort_order_record()->order[sort_index_b];
                    }
                    menu_sort_order_record()->order[sort_index_b] = work_index;
                    g_party_sort_marker.selected_idx = MENU_NONE;
                }
                else
                {
                    menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
                    g_party_sort_marker.selected_idx = MENU_NONE;
                }
            }
            else if (g_pad_input & PAD_BTN_SQUARE)
            {
                menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
                menu_sort_order_record()->order[0] = 0;
                menu_sort_order_record()->order[1] = 1;
                menu_sort_order_record()->order[2] = 2;
                menu_sort_order_record()->order[3] = 3;
                menu_sort_order_record()->order[4] = 4;
                menu_sort_order_record()->order[5] = 5;
                menu_sort_order_record()->order[6] = 6;
                menu_sort_order_record()->order[7] = 7;
            }
        }
        else if (g_pad_input & (PAD_BTN_CROSS | PAD_BTN_L3))
        {
            item_packed_x = content_items[g_menu_hit_item_idx].packed_x;
            item_type = item_packed_x & MENU_CONTENT_ITEM_TYPE_MASK;
            if (item_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
            {
                g_menu_active_subtype = content_items[g_menu_hit_item_idx].action_type;
                content_type = content_items[g_menu_hit_item_idx].action_type;
                switch (content_type)
                {
                case 1:
                case 2:
                    if (g_menu_char_slot == 0)
                    {
                        rect.x = 0x40;
                        rect.y = 0x60;
                        rect.w = 0xF0;
                        rect.h = 0x60;
                        submenu_slot = menu_slot_alloc(3, &rect);
                        submenu_slot->content_cb = (s32 * (*)()) & menu_spell_list_callback;
                        submenu_slot->navigation.packed =
                            (submenu_slot->navigation.packed & 0xFE00FFFF) | ((menu_build_spell_nav_entries() & MENU_ITEM_NAV_INDEX_MASK) << 16);
                        submenu_slot->has_title = 1;
                        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
                    }
                    break;

                case 3:
                case 4:
                case 5:
                case 6:
                    if (g_menu_char_slot == 0)
                    {
                        { u8* pad_base0 = (u8*)g_pad_ctx; item_flag = *(pad_base0 + content_type + 0x609); }
                        if ((item_flag != MENU_NONE) && (item_flag & 0x80))
                        {
                            rect.x = 0xB0;
                            rect.y = 0x60;
                            rect.w = 0x70;
                            rect.h = 0x50;
                        }
                        else
                        {
                            rect.x = 0xB0;
                            rect.y = 0x60;
                            rect.w = 0x70;
                            rect.h = 0x40;
                        }
                        submenu_slot = menu_slot_alloc(3, &rect);
                        submenu_slot->content_cb = (s32 * (*)()) & menu_subtype_action_callback;
                        { u8* pad_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250); item_flag = *(pad_base + content_type + 0x609); }
                        if ((item_flag != MENU_NONE) && (item_flag & 0x80))
                        {
                            submenu_slot->navigation.packed = (submenu_slot->navigation.packed & 0xFE00FFFF) | 0x40000;
                            menu_init_item_nav_entries(4);
                        }
                        else
                        {
                            submenu_slot->navigation.packed = (submenu_slot->navigation.packed & 0xFE00FFFF) | 0x30000;
                            menu_init_item_nav_entries(3);
                        }
                        scroll_list_update_target((ScrollListState*)submenu_slot, g_menu_scroll_nav_entries);

                        {
                            u8* equipped_item;
                            s32* active_equipped_item = &g_menu_active_equipped_item;

                            equipped_item = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((content_type << 6) + 0x90);
                            *active_equipped_item = (s32)equipped_item;
                            g_menu_item_ptr = (s32)equipped_item;
                            g_menu_saved_category0_item = g_menu_item_ptr;
                            g_menu_category0_item = g_menu_item_ptr;
                            g_menu_saved_category1_item = g_menu_item_ptr;
                            g_menu_category1_item = g_menu_item_ptr;
                            g_menu_saved_equipment_item = g_menu_item_ptr;
                            g_menu_category2_item = g_menu_item_ptr;
                        }
                        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
                    }
                    break;

                case 7:
                case 8:
                case 9:
                case 10:
                    if (g_menu_char_slot == 0)
                    {
                        rect.x = 0xB0;
                        rect.y = 0x30;
                        rect.w = 0x70;
                        rect.h = 0x60;
                        submenu_slot = menu_slot_alloc(3, &rect);
                        submenu_slot->content_cb = (s32 * (*)()) & menu_equipment_action_callback;
                        submenu_slot->navigation.packed = (submenu_slot->navigation.packed & 0xFE00FFFF) | 0x50000;
                        menu_init_item_nav_entries(5);
                        {
                            u8* equipped_item;
                            s32* active_equipped_item = &g_menu_active_equipped_item;

                            equipped_item = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((content_type << 6) - 0x170);
                            *active_equipped_item = (s32)equipped_item;
                            g_menu_saved_category0_item = (s32)equipped_item;
                            g_menu_saved_category1_item = (s32)equipped_item;
                            g_menu_saved_equipment_item = (s32)equipped_item;
                            menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
                        }
                    }
                    break;

                case 15:
                    if ((g_menu_scene_type >= 0x12) || (g_menu_scene_type == 1) || (g_menu_scene_type == 2))
                    {
                        if (((void*)g_menu_item_ptr != NULL) && (*((u8*)g_menu_item_ptr) != 0))
                        {
                            D_8011F424 = (((u32)((MenuItemEntry*)g_menu_item_ptr)->attributes.packed) >> 8) & 3;
                            func_800A8E28(&D_801226F0, g_menu_item_ptr);
                            text_buffer[0] = 0;
                            D_801226B8 = 0;
                            D_801227D4 = (void*)g_menu_item_ptr;
                            if (menu_item_is_nondefault((const MenuItemEntry*)g_menu_item_ptr) != 0)
                                {
                                text_cursor = text_buffer;
                                label_table = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                func_800A8E28(text_cursor, (s8*)(label_table + (*((u16*)(label_table + (((((MenuItemEntry*)g_menu_item_ptr)->attributes.halves.high) & 0x3F) * 2) + 0x48)))));
                                item_word = ((MenuItemEntry*)g_menu_item_ptr)->attributes.packed;
                                text_variant = (item_word >> 8) & 3;
                                switch (text_variant)
                                {
                                case 0:
                                    name_table = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(&D_801226B8, text_buffer, (s8*)(name_table + (*((u16*)(((item_word >> 9) & 0x7E) + (s32)name_table)))));
                                    break;

                                case 1:
                                    description_table = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(&D_801226B8, text_buffer, (s8*)(description_table + (*((u16*)(((item_word >> 9) & 0x7E) + (s32)description_table + 0x20)))));
                                    break;

                                default:
                                    detail_table = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(
                                        &D_801226B8, text_buffer,
                                        (s8*)(detail_table + (*((u16*)((((((u32)((MenuItemEntry*)g_menu_item_ptr)->attributes.packed) >> 9) & 0x7E)) + (s32)detail_table + 0x40)))));
                                    break;
                                }
                                g_menu_load_request = 1;
                                D_801229F4 = g_script_repeat_last;
                                if (g_menu_scene_type == 1)
                                {
                                    g_menu_transition_code = 0xB;
                                }
                                else if (g_menu_scene_type == 2)
                                {
                                    g_menu_transition_code = 0xC;
                                }
                                else
                                {
                                    g_menu_transition_code = 1;
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else if (item_type == MENU_CONTENT_ITEM_TYPE_ACTION)
            {
                action_group = g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                action_code = g_menu_content_action_codes[action_group][(item_packed_x >> 9) & 7];
                if (action_code != 0)
                {
                    menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
                    switch (action_code)
                    {
                    case 1:
                        {
                            u8* rec;
                            s32 idx;
                            s32 count;
                            u8* slot_base;

                            slot_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                            if (slot_base[0x5F0] != 0)
                            {
                                if ((slot_base[0x608] & 0x7F) == 4)
                                {
                                    D_801229F4 = slot_base[0x609] + 0x80;
                                }
                                else
                                {
                                    D_801229F4 = slot_base[0x609];
                                }
                                func_800A8E28(&D_801226F0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0));
                                count = 0;
                                idx = 0;
                                rec = (u8*)&D_800FD818;
                                D_801227D4 = (void*)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0));
                                for (; idx < 3; idx++)
                                {
                                    if (idx == g_menu_char_slot)
                                    {
                                        break;
                                    }
                                    if (*rec & 1)
                                    {
                                        count += 1;
                                    }
                                    rec += 0x268;
                                }
                                D_8011F424 = count + 3;
                                g_menu_load_request = 1;
                                g_menu_transition_code = 3;
                            }
                        }
                        break;

                    case 6:
                        g_pad_ctx->menu_option_flags |= MENU_OPTION_AUDIO_ENABLED;
                        akao_set_paused(0);
                        break;

                    case 7:
                        g_pad_ctx->menu_option_flags &= ~MENU_OPTION_AUDIO_ENABLED;
                        akao_set_paused(1);
                        break;

                    case 8:
                        g_pad_ctx->menu_option_flags |= MENU_OPTION_VIBRATION_ENABLED;
                        actuator_state->ports[0].small_motor_command = 1;
                        actuator_state->ports[0].large_motor_command = MENU_SE_VOLUME;
                        if (g_pad_ctx->inject_flags & MENU_PAD_INJECT_ENABLED)
                        {
                            actuator_state->ports[1].small_motor_command = 1;
                            actuator_state->ports[1].large_motor_command = MENU_SE_VOLUME;
                        }
                        break;

                    case 9:
                        g_pad_ctx->menu_option_flags &= ~MENU_OPTION_VIBRATION_ENABLED;
                        actuator_state->ports[0].small_motor_command = 0;
                        actuator_state->ports[1].small_motor_command = 0;
                        break;

                    case 10:
                        if (actuator_state->ports[1].padding0[0] == MENU_NONE)
                        {
                            g_menu_message_line1 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 101);
                            g_menu_message_line2 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 102);
                            menu_clear_slots();
                            menu_open_content_page(7);
                        }
                        else
                        {
                            inject_flags = &g_pad_ctx->inject_flags;
                            *inject_flags |= MENU_PAD_INJECT_ENABLED;
                            g_menu_companion_node = 0x2B;
                            menu_set_active_node();
                        }
                        break;

                    case 11:
                        inject_flags = &g_pad_ctx->inject_flags;
                        *inject_flags &= ~MENU_PAD_INJECT_ENABLED;
                        g_menu_companion_node = MENU_NONE;
                        menu_set_active_node();
                        break;
                    }
                }
            }
        }
        else if (g_pad_input & PAD_BTN_CIRCLE)
        {
            menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
            if ((u32)g_menu_scene_type >= 0x11U || (u8)(node_self_index = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx) < 0x14 || (u8)node_self_index >= 0x1C)
            {
                menu_reset_content_view();
            }
            else
            {
                if ((u32)(node_self_index - 0x14) < 5)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 1;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = (g_menu_char_slot * 3) + 3;
                    menu_focus_active_content_item();
                }
                else if ((u32)(node_self_index - 0x1A) < 2)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = 2;
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = (g_menu_char_slot * 3) + 3;
                    menu_focus_active_content_item();
                }
            }
        }

        work_index = 0;
        content_type = PAD_BTN_UP;
        for (; work_index < MENU_CONTENT_DIRECTION_COUNT; work_index++)
        {
            if (g_pad_input & content_type)
            {
                break;
            }
            content_type <<= 1;
        }

        if (work_index != MENU_CONTENT_DIRECTION_COUNT)
        {
            menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
            if (content_items[g_menu_hit_item_idx].params[work_index] == MENU_NONE)
            {
                if ((u32)g_menu_scene_type >= 0x11U || g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x14 ||
                    g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx >= 0x1C)
                {
                    s32* view_y_ptr = &g_content_view_y;
                    g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT_EXIT;
                    node_base = g_menu_nodes;
                    selected_node = &node_base[g_menu_active_node];
                    { s32 np = (selected_node->idx_nav.nav_x_packed >> 15) & 1; s32 ny = selected_node->u8_u.s.nav_y_hi; node_nav_y = (ny << 1) | np; }
                    scroll_extent = g_menu_content_height - 12;
                    *view_y_ptr = node_nav_y - scroll_extent;
                    if (*view_y_ptr < 12)
                    {
                        *view_y_ptr = MENU_CURSOR_Y_MIN;
                    }
                    if (*view_y_ptr >= MENU_CURSOR_Y_MAX)
                    {
                        *view_y_ptr = MENU_CURSOR_Y_MAX;
                    }
                    g_menu_suppress_cursor = 5;
                    g_content_view_x = ((selected_node->idx_nav.nav_x_packed >> 8) & 0x7F) + 8;
                }
            }
            else if (content_items[g_menu_hit_item_idx].params[work_index] != 0)
            {
                if (g_menu_scene_type < 0x11)
                {
                    if ((content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_ITEM_TYPE_MASK) == MENU_CONTENT_ITEM_TYPE_SUBMENU)
                    {
                        if (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x11)
                        {
                            g_menu_item_ptr = (s32)NULL;
                            g_menu_category0_item = (s32)NULL;
                            g_menu_category2_item = (s32)NULL;
                            g_menu_category1_item = (s32)NULL;
                        }
                    }
                }
                g_menu_hit_item_idx = content_items[g_menu_hit_item_idx].params[work_index];
                g_content_view_x = content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_X_MASK;
                g_content_view_y = content_items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET;
                g_menu_suppress_cursor = 3;
            }
        }
    }
}

/**
 * @brief Clamp the content cursor and snap the viewport to its position.
 */
void menu_snap_view_to_cursor(void)
{
    if (g_content_cursor_y < MENU_CURSOR_Y_MIN)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
    }
    if (g_content_cursor_y >= MENU_CURSOR_Y_MAX)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MAX;
    }
    g_content_view_y = g_content_cursor_y;

    /* Restore X from the active node's packed navigation column. */
    g_content_cursor_x =
        menu_nav_x(g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
    g_content_view_x = g_content_cursor_x;
}


/**
 * @brief Return non-zero if the item currently under the cursor has a confirm action.
 * @return 1 if the item will trigger a sub-menu or named action on confirm, 0 otherwise.
 */
s32 menu_item_has_action(void)
{
    MenuContentItem* items;
    MenuContentItem* item;
    u16 item_type;
    s16 item_subtype;
    s32 action_code;

    items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
    if ((g_menu_scene_type == 0x1F) || (g_menu_scene_type == 0x2B))
    {
        if (((int)g_menu_hit_item_idx) >= 0x11)
        {
            if (((int)g_menu_hit_item_idx) < 0x19)
            {
                return 1;
            }
        }
    }

    item = (MenuContentItem*)((g_menu_hit_item_idx * sizeof(MenuContentItem)) + (u32)items);
    item_type = item->packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

    if (item_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
    {
        g_menu_active_subtype = item->action_type;
        item_subtype = item->action_type;
        if (item_subtype != 0)
        {
            if (item_subtype >= 11)
            {
                if (item_subtype == 15)
                {
                    goto success;
                }
                return 0;
            }
            else
            {
                return g_menu_char_slot == 0;
            }
        }
    }
    else if (item_type == MENU_CONTENT_ITEM_TYPE_ACTION)
    {
        action_code = g_menu_content_action_codes[g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx]][(item->packed_x >> 9) & 7];
        if (action_code == 0)
        {
        }
        else if (action_code == 1)
        {
            return 1;
        }
        else if ((action_code != 0) && (action_code < 12) && (action_code >= 6))
        {
success:
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Exit content focus and restore the node-tree viewport to the active node.
 */
void menu_reset_content_view(void)
{
    MenuNode* active_node;
    s32 node_y;
    s32 node_y_low_bit;
    s32 scroll_offset;
    s32* view_y_ptr;

    if (g_menu_nodes[g_menu_scene_type].content_id != MENU_NONE)
    {
        g_content_cursor_x = g_menu_default_view_pos.x;
        g_content_cursor_y = g_menu_default_view_pos.y;
    }

    g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT_EXIT;
    view_y_ptr = &g_content_view_y;
    active_node = &g_menu_nodes[g_menu_active_node];
    node_y_low_bit = active_node->idx_nav.nav_x_packed >> 15;
    node_y = (active_node->u8_u.s.nav_y_hi << 1) | node_y_low_bit;

    /* Convert the node's layout position into the scrolled viewport. */
    scroll_offset = g_menu_content_height - MENU_CURSOR_Y_MIN;
    *view_y_ptr = node_y - scroll_offset;
    if (*view_y_ptr < MENU_CURSOR_Y_MIN)
    {
        *view_y_ptr = MENU_CURSOR_Y_MIN;
    }
    if (*view_y_ptr >= MENU_CURSOR_Y_MAX)
    {
        *view_y_ptr = MENU_CURSOR_Y_MAX;
    }

    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
    g_content_view_x = menu_nav_x(active_node->idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
}

/**
 * @brief Initialize packed positions and circular links for item navigation.
 * @param count Number of entries to initialize (no-op if <= 0).
 */
void menu_init_item_nav_entries(s32 count)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 entry_index;
    s32 wrapped_next_index;
    s32* entry;
    s32 packed_entry;
    s32 entry_with_previous;

    for (entry_index = 0; entry_index < count; entry_index++)
    {
        entry = entry_index + g_menu_item_nav_entries;
        packed_entry = *entry;
        previous_index = entry_index - 1;

        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;

        next_index = entry_index + 1;
        wrapped_next_index = 0;
        if (next_index < count)
        {
            wrapped_next_index = next_index;
        }
        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
}

/**
 * @brief Build circular navigation entries for the spell grid.
 * @return Number of present grid cells.
 */
s32 menu_build_spell_nav_entries(void)
{
    s32 presence_bits;
    s32 next_index;
    u32 entry_with_position;
    s32 index;
    s32 previous_index;
    s32 row;
    s32 item_count;
    s32 bit_mask;
    s32 wrapped_next_index;
    u32* entry;
    u8* presence_rows;
    u32 packed_entry;
    u32 entry_with_previous;

    item_count = 0;
    presence_rows = (u8*)g_pad_ctx + MENU_SPELL_GRID_OFFSET;

    for (row = MENU_SPELL_GRID_ROW_COUNT - 1; row >= 0; row--)
    {
        bit_mask = 1;
        presence_bits = *presence_rows;
        for (index = MENU_SPELL_GRID_COLUMN_COUNT - 1; index >= 0; index--)
        {
            if (presence_bits & bit_mask)
            {
                item_count++;
            }
            bit_mask <<= 1;
        }

        presence_rows += 1;
    }

    g_menu_scroll_nav_entries[0] = 0;
    for (index = 0; index < item_count; index++)
    {
        entry = g_menu_scroll_nav_entries + index;
        packed_entry = *entry;
        previous_index = index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;

        next_index = index + 1;
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
 * @brief Build navigation entries for the learned Special Technique list.
 * @return Item count in the low halfword; initial selected row in the high halfword.
 */
s32 menu_build_special_technique_nav_entries(void)
{
    s32 group_index;
    s32 index;
    s32 previous_index;
    s32 next_index;
    s32 item_count;
    s32 wrapped_next_index;
    s32 entry_with_previous;
    s32 selected_index;
    s32 none_index;
    s32 bit_mask;
    s32 group_flags;
    s32* group_flags_ptr;

    item_count = 0;
    selected_index = MENU_NONE;
    group_index = 0;
    none_index = selected_index;
    group_flags_ptr = (s32*)((u8*)g_pad_ctx + MENU_SPECIAL_TECHNIQUE_FLAGS_OFFSET);
    do
    {
        bit_mask = 1;
        group_flags = *group_flags_ptr;
        index = MENU_SPECIAL_TECHNIQUES_PER_GROUP - 1;

        do
        {
            if ((group_flags & bit_mask) != 0)
            {
                if ((group_index ==
                     (s32)(((u32)(((MenuItemEntry*)g_menu_equipment_base)->attributes.packed) >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK)) &&
                    (selected_index == none_index))
                {
                    selected_index = item_count;
                }
                item_count += 1;
            }
            index -= 1;
            bit_mask <<= 1;
        } while (index >= 0);
        group_index += 1;
        group_flags_ptr += 1;
    } while (group_index < MENU_SPECIAL_TECHNIQUE_GROUP_COUNT);

    if (selected_index == MENU_NONE)
    {
        selected_index = 0;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (index = 0; index < item_count; index++)
    {
        s32* nav_entry = (s32*)g_menu_scroll_nav_entries + index;
        s32 previous_entry = *nav_entry;
        s32 entry_with_position;

        previous_index = index - 1;
        entry_with_position = previous_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position |= (index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK;
        *nav_entry = entry_with_position;
        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *nav_entry = entry_with_previous;
        next_index = index + 1;
        wrapped_next_index = 0;
        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }
        *nav_entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return item_count | (selected_index << 16);
}

/**
 * @brief Build circular navigation entries for inventory items of one kind.
 * @param item_kind Item kind to match against MenuItemEntry::attributes.
 * @return Number of matching inventory items.
 */
s32 menu_build_inventory_nav_entries(s32 item_kind)
{
    s32 entry_index;
    s32 previous_index;
    s32 next_index;
    s32 item_count;
    s32 wrapped_next_index;
    s32 entry_with_previous;
    MenuItemEntry* item;

    entry_index = 0;
    item_count = 0;
    item = (MenuItemEntry*)((u8*)g_pad_ctx + MENU_ITEM_TABLE_OFFSET);
    for (; entry_index < MENU_ITEM_TABLE_COUNT; entry_index++, item++)
    {
        u8 active = item->active;

        if (active == 0)
        {
            break;
        }
        if (((item->attributes.packed >> MENU_ITEM_KIND_SHIFT) & 3) == (u32)item_kind)
        {
            item_count += 1;
        }
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (entry_index = 0; entry_index < item_count; entry_index++)
    {
        s32* nav_entry = (s32*)g_menu_scroll_nav_entries + entry_index;
        s32 previous_entry = *nav_entry;
        s32 entry_with_position;

        previous_index = entry_index - 1;
        entry_with_position = previous_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position |= (entry_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK;
        *nav_entry = entry_with_position;
        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *nav_entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;
        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }
        *nav_entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return item_count;
}

/**
 * @brief Return a saved-history record from the shared context.
 * @param context Context containing the saved-history records.
 * @param index Record index.
 * @return Selected history record.
 */
static inline LargeHistoryRecord* menu_history_record(PadContext* context, s32 index)
{
    u8* records = (u8*)context->large_history_records;
    return (LargeHistoryRecord*)(records + index * sizeof(LargeHistoryRecord));
}

/**
 * @brief Read one packed value from the active character's menu data.
 * @param context Shared context containing the character records.
 * @param index Value index, 0 through 7.
 * @return Packed value; the character panel displays its high seven bits.
 */
static inline u16 menu_active_character_value(PadContext* context, s32 index)
{
    s32 offset = index * sizeof(u16);
    offset += g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return *(u16*)((u8*)context + offset + MENU_CHARACTER_PACKED_VALUES_OFFSET);
}

/**
 * @brief Return the active character's record in the shared context.
 * @param context Context containing the character records.
 * @return Active character record.
 */
static inline MenuCharacterRecord* menu_active_character(PadContext* context)
{
    s32 offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return (MenuCharacterRecord*)((u8*)context + offset + MENU_CHARACTER_RECORD_OFFSET);
}

/* ----- Scene Content Rendering ----- */

/**
 * @brief Draw the active scene's content entries.
 * @param packet_cursor GPU packet cursor advanced as primitives are emitted.
 * @param ot_entry Ordering-table entry passed to render helpers.
 * @return Updated GPU packet cursor after drawing.
 */
void* menu_draw_scene_content(void* packet_cursor, s32* ot_entry)
{
    u8 text_buffer[0x40];
    u8 prefix_buffer[0x40];
    Vec2s pos;
    u8 label_buffer[16];
    s32 remaining_count;
    MenuContentItem* content_item;
    u8* draw_source;
    void* draw_packet_cursor;
    void *base_a2_0, *base_a2_1, *base_a2_2, *base_a2_3, *base_a2_4, *base_a2_5, *base_a2_6, *base_a2_7, *base_a2_8, *base_a2_9, *base_a2_10, *base_a2_11,
        *base_a2_12;
    s32 k;
    u32 shared_s0;
    s32 content_index;
    s32 two_outer = 2;
    s32 val;
    s32 zero = 0;

    if (g_menu_scene_type == -1)
    {
        content_item = &g_menu_default_content_items;
        remaining_count = g_menu_default_content_count_minus_one + 1;
    }
    else
    {
        u8 idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
        content_item = g_menu_content_table[idx];
        remaining_count = g_menu_content_item_counts[g_menu_content_group_ids[idx]];
    }

    if (content_item != NULL)
    {
        if (content_item == (MenuContentItem*)1)
        {
            g_menu_load_request = (s32)content_item;
        }
        else
        {
            u8* character_equipment_area;
            MenuItemEntry* equipped_items;
            g_menu_equipment_base = (u32)(equipped_items = (MenuItemEntry*)((character_equipment_area = (u8*)g_pad_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE + MENU_CHARACTER_RECORD_OFFSET)) + MENU_CHARACTER_ITEMS_OFFSET));
            D_80168C30 = (u8*)equipped_items;
            D_80168C20 = (u8*)equipped_items;
            D_80168C24 = (u8*)equipped_items;
            if (g_menu_scene_type == 0x10)
            {
                MenuItemEntry* second_equipped_item;
                g_menu_category0_item = (u32)equipped_items;
                g_menu_category1_item = (u32)(second_equipped_item = &equipped_items[1]);
                g_menu_category2_item = (u32)second_equipped_item;
            }

            if (remaining_count != 0)
            {
                do
                {
                    pos.x = (s16)(content_item->packed_x & MENU_CONTENT_X_MASK);
                    pos.y = (s16)(content_item->y - MENU_CONTENT_VIEW_Y_OFFSET);
                    {
                        s32 t0 = 0xFF;
                        u16 item_word = content_item->packed_x;
                        s32 item_type = item_word >> MENU_CONTENT_TYPE_SHIFT;
                        switch (item_type)
                        {
                        case 1:
                            draw_packet_cursor = packet_cursor;
                            base_a2_0 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES]);
                            {
                                s32 a3 = 1;
                                void* a2_2 = (void*)((u8*)base_a2_0 + *(u16*)((u8*)base_a2_0 + (content_item->action_type * 2)));
                                packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, a3, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                              (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                            }
                            break;

                        case 2:
                        {
                            s32 sub;
                            if (content_item->action_type < 2)
                            {
                                t0 = 0xFF;
                                {
                                    s32 y = two_outer;
                                    if (y == 2)
                                    {
                                        t0 = 0x1E;
                                    }
                                }
                            }
                            {
                                u8 sub_byte = content_item->action_type;
                                sub = sub_byte;
                            }
                            if ((u32)(sub - 2) < 8)
                            {
                                s32 s;
                                val = 0;
                                if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                                {
                                    val = ((MenuItemEntry*)g_menu_category1_item)->effect_flags[0];
                                }
                                s = content_item->action_type;
                                if (((val >> (s - 2)) & 1) != 0)
                                {
                                    t0 = s + 0x3B;
                                }
                                else
                                {
                                    t0 = 0x21;
                                }
                            }
                            else if ((u32)(sub - 0xA) < 8)
                            {
                                s32 s;
                                val = 0;
                                if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                                {
                                    val = ((MenuItemEntry*)g_menu_category1_item)->effect_flags[1];
                                }
                                t0 = 0x21;
                                s = content_item->action_type;
                                if (((val >> (s - 0xA)) & 1) != 0)
                                {
                                    t0 = s + 0x2B;
                                }
                            }
                            else if ((u32)(sub - 0x12) < 8)
                            {
                                s32 s;
                                val = 0;
                                if (((MenuItemEntry*)D_80168C20)[1].active != 0)
                                {
                                    val = ((MenuItemEntry*)D_80168C20)[1].effect_flags[0];
                                }
                                if (((MenuItemEntry*)D_80168C20)[2].active != 0)
                                {
                                    val |= ((MenuItemEntry*)D_80168C20)[2].effect_flags[0];
                                }
                                if (((MenuItemEntry*)D_80168C20)[3].active != 0)
                                {
                                    val |= ((MenuItemEntry*)D_80168C20)[3].effect_flags[0];
                                }
                                t0 = 0x21;
                                s = content_item->action_type;
                                if (((val >> (s - 0x12)) & 1) != 0)
                                {
                                    t0 = s + 0x2B;
                                }
                            }
                            else if ((u32)(sub - 0x1A) < 8)
                            {
                                s32 s;
                                val = 0;
                                if (((MenuItemEntry*)D_80168C20)[1].active != 0)
                                {
                                    val = ((MenuItemEntry*)D_80168C20)[1].effect_flags[1];
                                }
                                if (((MenuItemEntry*)D_80168C20)[2].active != 0)
                                {
                                    val |= ((MenuItemEntry*)D_80168C20)[2].effect_flags[1];
                                }
                                if (((MenuItemEntry*)D_80168C20)[3].active != 0)
                                {
                                    val |= ((MenuItemEntry*)D_80168C20)[3].effect_flags[1];
                                }
                                s = content_item->action_type;
                                if (((val >> (s - 0x1A)) & 1) != 0)
                                {
                                    t0 = D_80168696[s];
                                }
                                else
                                {
                                    t0 = 0x21;
                                }
                            }
                            else if ((u32)(sub - 0x22) < 8)
                            {
                                t0 = 0x21;
                                if (g_menu_category0_item != 0)
                                {
                                    s32 s;
                                    s = content_item->action_type;
                                    if (((((MenuItemEntry*)g_menu_category0_item)->effect_flags[0] >> (s - 0x22)) & 1) != 0)
                                    {
                                        t0 = s + 0x13;
                                    }
                                }
                            }
                            else if ((u32)(sub - 0x2A) < 8)
                            {
                                s32 idx;
                                s32 s;
                                s = content_item->action_type;
                                for (idx = 0; idx < 8; idx++)
                                {
                                    u8* pad = (u8*)g_pad_ctx;
                                    s32 character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                                    if (*(u8*)(pad + character_offset + idx + 0x638) == s - 0x2A)
                                    {
                                        break;
                                    }
                                }
                                t0 = D_801686B8[idx];
                            }
                            else if ((u32)(sub - 0x32) < 4)
                            {
                                if (g_menu_char_slot != 2)
                                {
                                    MenuItemEntry* equipment = (MenuItemEntry*)((content_item->action_type << 6) + (s32)g_menu_equipment_base - 0xC80);
                                    t0 = 0x21;
                                    if (equipment->active != 0)
                                    {
                                        u32 attributes = equipment->attributes.packed;
                                        s32 item_kind = (attributes >> MENU_ITEM_KIND_SHIFT) & 3;
                                        switch (item_kind)
                                        {
                                        case 1:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x50;
                                            break;
                                        case 2:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x5C;
                                            break;
                                        case 0:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x45;
                                            break;
                                        }
                                    }
                                }
                            }
                            else if ((u8)sub == 0x36)
                            {
                                t0 = 0xFF;
                                if (g_menu_item_ptr != 0)
                                {
                                    if (((MenuItemEntry*)g_menu_item_ptr)->active != 0)
                                    {
                                        u32 attributes = ((MenuItemEntry*)g_menu_item_ptr)->attributes.packed;
                                        s32 item_kind = (attributes >> MENU_ITEM_KIND_SHIFT) & 3;
                                        switch (item_kind)
                                        {
                                        case 0:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x45;
                                            break;
                                        case 1:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x50;
                                            break;
                                        case 2:
                                            t0 = ((attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK) + 0x5C;
                                            break;
                                        }
                                    }
                                }
                            }
                            else if ((u32)(sub - 0x37) < 0x1E)
                            {
                                t0 = D_80168659[content_item->action_type];
                            }
                            else if ((u8)sub == 0x55)
                            {
                                t0 = 0xFF;
                                if (((g_pad_ctx->menu_option_flags >> 1) & 1) != 0)
                                {
                                    t0 = 0x6A;
                                }
                            }
                            else
                            {
                                sub = content_item->action_type;
                                if (sub == 0x56)
                                {
                                    t0 = 0x6A;
                                    if (((g_pad_ctx->menu_option_flags >> 1) & 1) != 0)
                                    {
                                        t0 = 0xFF;
                                    }
                                }
                                else if (sub == 0x57)
                                {
                                    t0 = 0xFF;
                                    if ((g_pad_ctx->menu_option_flags & 1) != 0)
                                    {
                                        t0 = 0x6A;
                                    }
                                }
                                else if (sub == 0x58)
                                {
                                    t0 = 0x6A;
                                    if ((g_pad_ctx->menu_option_flags & 1) != 0)
                                    {
                                        t0 = 0xFF;
                                    }
                                }
                                else if (sub == 0x59)
                                {
                                    t0 = 0xFF;
                                    if ((g_pad_ctx->inject_flags & 0x80) != 0)
                                    {
                                        t0 = 0x6A;
                                    }
                                }
                                else if (sub == 0x5A)
                                {
                                    t0 = 0x6A;
                                    if ((g_pad_ctx->inject_flags & 0x80) != 0)
                                    {
                                        t0 = 0xFF;
                                    }
                                }
                                else
                                {
                                    t0 = 0xFF;
                                }
                            }
                            if (t0 != 0xFF)
                            {
                                packet_cursor = menu_emit_draw_mode_primitive(
                                    menu_emit_icon_sprite(packet_cursor, ot_entry, t0, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET, 0, 0, 0, 0),
                                    ot_entry);
                            }
                        }
                        break;

                        case 3:
                        {
                            content_index = content_item->action_type;
                            switch (content_index)
                            {
                            case 0x1:
                                if (g_menu_item_ptr != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    draw_source = (void*)g_menu_item_ptr;
                                    packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, draw_source, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                                break;
                            case 0x2:
                                if (g_menu_item_ptr != 0)
                                {
                                    u32 v;
                                    u32 v1;

                                    if (menu_item_is_nondefault((const MenuItemEntry*)g_menu_item_ptr) != 0)
                                    {
                                        void* a3 =
                                            (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_KEY_ITEM_NAMES]);
                                        menu_concat_encoded_text(
                                            &prefix_buffer,
                                            (void*)((u8*)a3 + *(u16*)((u8*)a3 + (((MenuItemEntry*)g_menu_item_ptr)->attributes.halves.high & 0x3F) * 2)),
                                            (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] +
                                                    *(u16*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] +
                                                            0xB4)));
                                    }
                                    else
                                    {
                                        prefix_buffer[0] = 0;
                                    }
                                    v = ((MenuItemEntry*)g_menu_item_ptr)->attributes.packed;
                                    v1 = (v >> 8) & 3;
                                    switch (v1)
                                    {
                                    case 0:
                                    {
                                        void* base0 = (void*)((u8*)g_menu_state_ptr +
                                                              ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP]);
                                        menu_concat_encoded_text(&text_buffer, &prefix_buffer,
                                                                 (void*)((u8*)base0 + *(u16*)((s32)((v >> 9) & 0x7E) + (s32)base0)));
                                        break;
                                    }
                                    case 1:
                                    {
                                        void* base1 = (void*)((u8*)g_menu_state_ptr +
                                                              ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP]);
                                        menu_concat_encoded_text(&text_buffer, &prefix_buffer,
                                                                 (void*)((u8*)base1 + *(u16*)((s32)((v >> 9) & 0x7E) + (s32)base1 + 0x16)));
                                        break;
                                    }
                                    default:
                                    {
                                        void* base2 = (void*)((u8*)g_menu_state_ptr +
                                                              ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP]);
                                        menu_concat_encoded_text(
                                            &text_buffer, &prefix_buffer,
                                            (void*)((u8*)base2 + *(u16*)((s32)((((MenuItemEntry*)g_menu_item_ptr)->attributes.packed >> 9) & 0x7E) + (s32)base2 + 0x2E)));
                                        break;
                                    }
                                    }
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &text_buffer, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                                break;
                            case 0x3:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x4:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed >> 4) & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x5:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed >> 8) & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x6:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed >> 12) & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x7:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.halfwords[1] & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x8:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed >> 20) & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x9:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.bytes[3] & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0xA:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (((MenuItemEntry*)g_menu_item_ptr)->display_nibbles.packed >> 28) & 0xF, 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0xB:
                            case 0xC:
                            case 0xD:
                                if (g_menu_item_ptr != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_1 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x5C));
                                    {
                                        s32 a3 = 1;
                                        void* a2_2 = (void*)((u8*)base_a2_1 + *(u16*)((u8*)base_a2_1 + (*(u8*)(g_menu_item_ptr + content_index + 0x15) * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, a3, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                                break;
                            case 0xE:
                            case 0xF:
                            case 0x10:
                                if (g_menu_category0_item != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                                    {
                                        s32 a3 = 1;
                                        void* a2_2 =
                                            (void*)((u8*)base_a2_2 + *(u16*)((u8*)base_a2_2 + (*(u8*)(g_menu_category0_item + content_index + 0x1A) * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, a3, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                                break;
                            case 0x11:
                                if (g_menu_category0_item != 0)
                                {
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_category0_item)->stat_values[0], 1, &pos,
                                                                  ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x12:
                                if (g_menu_category1_item != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_category1_item)->stat_values[0], 1, &pos,
                                                                             ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x13:
                                if (g_menu_item_ptr != 0)
                                {
                                    void* a2;
                                    u8* a3ptr;
                                    s32 idx;
                                    void* a2_2;
                                    draw_packet_cursor = packet_cursor;
                                    a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x44));
                                    a3ptr = (u8*)g_menu_category2_item;
                                    idx = (a3ptr[0x24] * 0xE + a3ptr[0x25]) * 2;
                                    a2_2 = (void*)((u8*)a2 + *(u16*)(idx + (u32)a2));
                                    packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                                break;
                            case 0x14:
                                if (g_menu_item_ptr != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_3 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x28));
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_3 + *(u16*)((u8*)base_a2_3 + (*(u8*)(g_menu_category2_item + 0x24) * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                                break;
                            case 0x15:
                                if (g_menu_item_ptr != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_4 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x38));
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_4 + *(u16*)((u8*)base_a2_4 + (*(u8*)(g_menu_category2_item + 0x25) * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                                break;
                            case 0x16:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, *(u8*)(g_menu_category2_item + 0x26), 1, &pos,
                                                                  ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x17:
                            case 0x18:
                            case 0x19:
                            case 0x1A:
                            {
                                val = 0;
                                if (g_item_slot_flags[content_index - 0x17] != 0)
                                {
                                    MenuItemEntry* equipment = (MenuItemEntry*)((content_index << 6) + (s32)D_80168C30 - 0x5C0);
                                    MenuItemEntry* comparison_item;
                                    if (equipment->active != 0)
                                    {
                                        val = equipment->stat_values[0];
                                    }
                                    comparison_item = (MenuItemEntry*)g_item_slot_data[content_index - 0x17];
                                    if (comparison_item != 0 && comparison_item->active != 0)
                                    {
                                        val -= comparison_item->stat_values[0];
                                    }
                                    else
                                    {
                                        val -= D_800F0C1C;
                                    }
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, abs(val), 1, &pos,
                                                                  ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            /* fallthrough */
                            case 0x1B:
                            case 0x1C:
                            case 0x1D:
                            case 0x1E:
                            {
                                val = 0;
                                if (g_item_slot_flags[content_index - 0x1B] != 0)
                                {
                                    MenuItemEntry* equipment = (MenuItemEntry*)((content_index << 6) + (s32)D_80168C30 - 0x6C0);
                                    MenuItemEntry* comparison_item;
                                    if (equipment->active != 0)
                                    {
                                        val = equipment->stat_values[0];
                                    }
                                    comparison_item = (MenuItemEntry*)g_item_slot_data[content_index - 0x1B];
                                    if (comparison_item != 0 && comparison_item->active != 0)
                                    {
                                        val -= comparison_item->stat_values[0];
                                    }
                                    else
                                    {
                                        val -= D_800F0C1C;
                                    }
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                            }
                            break;
                            case 0x1F:
                            case 0x20:
                            case 0x21:
                            case 0x22:
                            case 0x23:
                            case 0x24:
                            case 0x25:
                            case 0x26:
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 res = menu_lookup_item_nibble(g_menu_item_ptr, content_index - 0x1F);
                                    menu_copy_sign_label(label_buffer, res);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                                break;
                            case 0x27:
                            case 0x28:
                            case 0x29:
                            case 0x2A:
                            case 0x2B:
                            case 0x2C:
                            case 0x2D:
                            case 0x2E:
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 diff = menu_lookup_item_nibble(g_menu_item_ptr, content_index - 0x27);
                                    packet_cursor =
                                        menu_draw_clamped_number(ot_entry, packet_cursor, (u32)abs(diff), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x2F:
                            case 0x30:
                            case 0x31:
                            case 0x32:
                            case 0x33:
                            case 0x34:
                            case 0x35:
                            case 0x36:
                            {
                                s32 idx2 = content_index - 0x2F;
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 v1 = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                                    s32 v2 = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                                    s32 diff = v1 - v2;
                                    menu_copy_sign_label(label_buffer, diff);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                            }
                            break;
                            case 0x37:
                            case 0x38:
                            case 0x39:
                            case 0x3A:
                            case 0x3B:
                            case 0x3C:
                            case 0x3D:
                            case 0x3E:
                            {
                                s32 idx2 = content_index - 0x2F;
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 v1 = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                                    s32 v2 = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                                    s32 diff = v1 - v2;
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, abs(diff), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            break;
                            case 0x3F:
                            case 0x40:
                            case 0x41:
                            case 0x42:
                                if (g_menu_category1_item != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(
                                        ot_entry, packet_cursor, ((MenuItemEntry*)g_menu_category1_item)->stat_values[content_index - 0x3F], 1, &pos,
                                        ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 0x43:
                            case 0x44:
                            case 0x45:
                            case 0x46:
                            {
                                s32 has = 0;

                                val = has;
                                for (k = 1; k < 4; k++)
                                {
                                    u8* slot = D_80168C20 + (k * 0x40);
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (((MenuItemEntry*)slot)->active != 0)
                                        {
                                            val += ((MenuItemEntry*)slot)->stat_values[content_index - 0x43];
                                        }
                                        has = g_item_slot_data[k];
                                        if (has != 0 && ((MenuItemEntry*)has)->active != 0)
                                        {
                                            val -= ((MenuItemEntry*)has)->stat_values[content_index - 0x43];
                                        }
                                        has = 1;
                                    }
                                }
                                if (has)
                                {
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                            }
                            break;
                            case 0x47:
                            case 0x48:
                            case 0x49:
                            case 0x4A:
                            {
                                s32 has = 0;
                                s32 total = has;

                                for (k = 1; k < 4; k++)
                                {
                                    u8* slot = D_80168C20 + (k * 0x40);
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (((MenuItemEntry*)slot)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)slot)->stat_values[content_index - 0x47];
                                        }
                                        has = g_item_slot_data[k];
                                        if (has != 0 && ((MenuItemEntry*)has)->active != 0)
                                        {
                                            total -= ((MenuItemEntry*)has)->stat_values[content_index - 0x47];
                                        }
                                        has = 1;
                                    }
                                }
                                if (has)
                                {
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, abs(total), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            break;
                            default:
                                break;
                            }
                        }
                        break;

                        case 4:
                        {
                            content_index = content_item->action_type;
                            switch (content_index)
                            {
                            case 0x1:
                            {
                                u8 u_val;
                                u8 v_val;
                                SET_BGR0_PACKED(packet_cursor, GPU_TINT_NEUTRAL);
                                setSprt(packet_cursor);
                                ((SPRT*)packet_cursor)->x0 = content_item->packed_x & MENU_CONTENT_X_MASK;
                                ((SPRT*)packet_cursor)->y0 = content_item->y - MENU_CONTENT_VIEW_Y_OFFSET;
                                u_val = 0xD0;
                                if (g_menu_char_slot == 2)
                                {
                                    u_val = 0xA0;
                                }
                                ((SPRT*)packet_cursor)->u0 = u_val;
                                v_val = 0x50;
                                if (g_menu_char_slot == 0)
                                {
                                    v_val = 0x20;
                                }
                                ((SPRT*)packet_cursor)->v0 = v_val;
                                SET_SPRT_WH_PACKED(packet_cursor, 0x30, 0x30);
                                SET_SPRT_CLUT(packet_cursor, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (SPRT*)packet_cursor + 1;
                                SET_BGR0_PACKED(packet_cursor, 0);
                                setSprt(packet_cursor);
                                setSemiTrans(packet_cursor, 1);
                                ((SPRT*)packet_cursor)->x0 = (content_item->packed_x & MENU_CONTENT_X_MASK) + 2;
                                ((SPRT*)packet_cursor)->y0 = content_item->y - 6;
                                u_val = 0xD0;
                                if (g_menu_char_slot == 2)
                                {
                                    u_val = 0xA0;
                                }
                                ((SPRT*)packet_cursor)->u0 = u_val;
                                v_val = 0x50;
                                if (g_menu_char_slot == 0)
                                {
                                    v_val = 0x20;
                                }
                                ((SPRT*)packet_cursor)->v0 = v_val;
                                SET_SPRT_WH_PACKED(packet_cursor, 0x30, 0x30);
                                SET_SPRT_CLUT(packet_cursor, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (SPRT*)packet_cursor + 1;
                                setDrawTPage(packet_cursor, 0, 0, 0x1F);
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (DR_TPAGE*)packet_cursor + 1;
                            }
                            break;
                            case 0x2:
                            {
                                u8* call_base = (u8*)g_pad_ctx;
                                s32 off = g_menu_char_slot * 0x250 + 0x5F0;
                                packet_cursor = func_800A88A0(packet_cursor, ot_entry, call_base + off, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                              (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                            }
                            break;
                            case 0x3:
                            {
                                PadContext* context = g_pad_ctx;
                                u8 v = menu_active_character(context)->progression.fields.level;
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, v, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x4:
                            {
                                u8* rec_base = (u8*)&D_80105AE0;
                                s32 rec_off = g_menu_char_slot * 0x23C;
                                packet_cursor =
                                    func_800A8A78(ot_entry, packet_cursor, *(s32*)(rec_base + rec_off + 4), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x5:
                            {
                                PadContext* context = g_pad_ctx;
                                u16 v = menu_active_character(context)->unknown_0x24;
                                packet_cursor = func_800A8A78(ot_entry, packet_cursor, v, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x6:
                            {
                                PadContext* context = g_pad_ctx;
                                u32 v = menu_active_character(context)->progression.packed;
                                packet_cursor = func_800A8A78(ot_entry, packet_cursor, (v >> 8), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x7:
                            case 0x8:
                            case 0x9:
                            case 0xA:
                            case 0xB:
                            case 0xC:
                            case 0xD:
                            case 0xE:
                            {
                                void* base = (void*)g_pad_ctx;
                                s32 idx = content_index - 7;
                                packet_cursor = menu_draw_clamped_number(
                                    ot_entry, packet_cursor,
                                    menu_active_character_value((PadContext*)base, idx) >> 9, 1,
                                    &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0xF:
                            {
                                void* a2_2 = (void*)g_menu_equipment_base;
                                if (*(u8*)a2_2 != 0)
                                {
                                    s32 a3 = 1;
                                    if (g_item_slot_flags[0] != 0)
                                    {
                                        a3 = 2;
                                    }
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, a2_2, a3, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x10:
                            case 0x11:
                            case 0x12:
                                if (((MenuItemEntry*)D_80168C30)->active != 0)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_5 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_5 + *(u16*)((u8*)base_a2_5 + (*(u8*)(D_80168C30 + content_index + 0x18) * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                                break;
                            case 0x13:
                            {
                                s32 displayed_stat = zero;
                                if (g_item_slot_flags[0] != 0)
                                {
                                    MenuItemEntry* comparison_item = (MenuItemEntry*)g_item_slot_data[0];
                                    if (comparison_item == 0 || comparison_item->active == 0)
                                    {
                                        displayed_stat += D_800F0C1C;
                                    }
                                    else
                                    {
                                        displayed_stat = comparison_item->stat_values[0];
                                    }
                                }
                                else
                                {
                                    if (((MenuItemEntry*)D_80168C30)->active != 0)
                                    {
                                        displayed_stat = ((MenuItemEntry*)D_80168C30)->stat_values[0];
                                    }
                                }
                                packet_cursor = func_800A8A78(ot_entry, packet_cursor, displayed_stat, 1, &pos,
                                                              ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x14:
                            case 0x15:
                            case 0x16:
                            {
                                u32 base = g_menu_equipment_base;
                                s32 shl = content_index << 6;
                                if (*(u8*)(shl + base - 0x4C0) != 0)
                                {
                                    s32 a3 = 1;
                                    s32 off = shl - 0x4C0;
                                    void* a2_2 = (void*)(base + off);
                                    if (g_item_slot_flags[content_index - 0x13] != 0)
                                    {
                                        a3 = 2;
                                    }
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, a2_2, a3, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x17:
                            case 0x18:
                            {
                                void* base;
                                u8 idx;
                                draw_packet_cursor = packet_cursor;
                                base_a2_6 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x10));
                                base = (void*)g_pad_ctx;
                                {
                                    u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + content_index + 0x5F3;
                                    idx = *idxp;
                                }
                                {
                                    void* a2_2 = (void*)((u8*)base_a2_6 + *(u16*)((u8*)base_a2_6 + (idx * 2)));
                                    packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x19:
                            {
                                s32 v = func_800B607C(g_menu_char_slot);
                                PadContext* context = g_pad_ctx;
                                u32 shift = menu_active_character(context)->progression.packed >> 8;
                                packet_cursor = func_800A8A78(ot_entry, packet_cursor, v - shift, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x1B:
                            case 0x1C:
                            case 0x1D:
                            case 0x1E:
                            {
                                void* base = (void*)g_pad_ctx;
                                u8* ptr;
                                u8 val;
                                ptr = (u8*)base + g_menu_char_slot * 0x250;
                                ptr += content_index;
                                content_index = (u32)ptr;
                                val = ((MenuNameSelection*)(ptr + 0x5F1))->packed;
                                if (val != 0xFF)
                                {
                                    draw_packet_cursor = packet_cursor;
                                    if (val & 0x80)
                                    {
                                        u16* lvar_v1_2 = (u16*)g_pad_ctx;
                                        void* lvar_a2 = (void*)((u8*)lvar_v1_2 + (g_menu_char_slot * 0x250 + 0x5F0));
                                        void* a2_2 = (void*)((u8*)lvar_a2 + ((((MenuNameSelection*)(ptr + 0x5F1))->fields.index << 6) + 0x150));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                    else
                                    {
                                        u32 tmpv;
                                        u8 idx;
                                        u16 off2;
                                        base_a2_7 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x20));
                                        tmpv = ((((MenuItemEntry*)g_menu_equipment_base)->attributes.packed >> 10) & 0x3F);

                                        idx = ((MenuNameSelection*)(ptr + 0x5F1))->fields.index;
                                        off2 = idx * 2;
                                        {
                                            void* a2_2 = (void*)((u8*)base_a2_7 + *(u16*)(off2 + (tmpv * 0x30 + (s32)base_a2_7)));
                                            packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                          (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                        }
                                    }
                                }
                            }
                            break;
                            case 0x1F:
                            {
                                void* base;
                                u8 idx;
                                draw_packet_cursor = packet_cursor;
                                base_a2_8 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x4C));
                                base = (void*)g_pad_ctx;
                                {
                                    u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + 0x609;
                                    idx = *idxp;
                                }
                                {
                                    void* a2_2 = (void*)((u8*)base_a2_8 + *(u16*)((u8*)base_a2_8 + (idx * 2)));
                                    packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x20:
                            {
                                u32 total = 0;
                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        u32 ptr = g_item_slot_data[k];
                                        if (ptr != 0 && ((MenuItemEntry*)ptr)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)ptr)->stat_values[0];
                                        }
                                    }
                                    else
                                    {
                                        u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                        if (((MenuItemEntry*)v)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)v)->stat_values[0];
                                        }
                                    }
                                }
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, total, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x21:
                            {
                                u32 total = 0;
                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        u32 ptr = g_item_slot_data[k];
                                        if (ptr != 0 && ((MenuItemEntry*)ptr)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)ptr)->stat_values[1];
                                        }
                                    }
                                    else
                                    {
                                        u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                        if (((MenuItemEntry*)v)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)v)->stat_values[1];
                                        }
                                    }
                                }
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, total, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x22:
                            {
                                u32 total = 0;
                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        u32 ptr = g_item_slot_data[k];
                                        if (ptr != 0 && ((MenuItemEntry*)ptr)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)ptr)->stat_values[2];
                                        }
                                    }
                                    else
                                    {
                                        u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                        if (((MenuItemEntry*)v)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)v)->stat_values[2];
                                        }
                                    }
                                }
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, total, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x23:
                            {
                                u32 total = 0;
                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        u32 ptr = g_item_slot_data[k];
                                        if (ptr != 0 && ((MenuItemEntry*)ptr)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)ptr)->stat_values[3];
                                        }
                                    }
                                    else
                                    {
                                        u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                        if (((MenuItemEntry*)v)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)v)->stat_values[3];
                                        }
                                    }
                                }
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, total, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            case 0x3F:
                            {
                                val = 0;
                                if (g_item_slot_flags[0] != 0)
                                {
                                    if (((MenuItemEntry*)D_80168C20)->active != 0)
                                    {
                                        val = ((MenuItemEntry*)D_80168C30)->stat_values[0];
                                    }
                                    if (g_item_slot_data[0] != 0 && *(u8*)g_item_slot_data[0] != 0)
                                    {
                                        val = val - ((MenuItemEntry*)g_item_slot_data[0])->stat_values[0];
                                    }
                                    else
                                    {
                                        val = val - D_800F0C1C;
                                    }
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                            }
                            break;
                            case 0x40:
                            case 0x41:
                            case 0x42:
                            case 0x43:
                            {
                                s32 has = 0;

                                val = has;
                                for (k = 1; k < 4; k++)
                                {
                                    u8* slot = D_80168C20 + (k * 0x40);
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (((MenuItemEntry*)slot)->active != 0)
                                        {
                                            val += ((MenuItemEntry*)slot)->stat_values[content_index - 0x40];
                                        }
                                        has = g_item_slot_data[k];
                                        if (has != 0 && ((MenuItemEntry*)has)->active != 0)
                                        {
                                            val -= ((MenuItemEntry*)has)->stat_values[content_index - 0x40];
                                        }
                                        has = 1;
                                    }
                                }
                                if (has)
                                {
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, &label_buffer, 1, pos.x, pos.y, 0);
                                }
                            }
                            break;
                            case 0x44:
                            {
                                u16 a2_23 = 0;
                                if (g_item_slot_flags[0] != 0)
                                {
                                    s32 diff;
                                    if (((MenuItemEntry*)D_80168C20)->active != 0)
                                    {
                                        a2_23 = ((MenuItemEntry*)D_80168C30)->stat_values[0];
                                    }
                                    if (g_item_slot_data[0] != 0 && *(u8*)g_item_slot_data[0] != 0)
                                    {
                                        diff = a2_23 - ((MenuItemEntry*)g_item_slot_data[0])->stat_values[0];
                                    }
                                    else
                                    {
                                        diff = a2_23 - D_800F0C1C;
                                    }
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, abs(diff), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            break;
                            case 0x45:
                            case 0x46:
                            case 0x47:
                            case 0x48:
                            {
                                s32 has = 0;
                                s32 total = has;

                                for (k = 1; k < 4; k++)
                                {
                                    u8* slot = D_80168C20 + (k * 0x40);
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (((MenuItemEntry*)slot)->active != 0)
                                        {
                                            total += ((MenuItemEntry*)slot)->stat_values[content_index - 0x45];
                                        }
                                        has = g_item_slot_data[k];
                                        if (has != 0 && ((MenuItemEntry*)has)->active != 0)
                                        {
                                            total -= ((MenuItemEntry*)has)->stat_values[content_index - 0x45];
                                        }
                                        has = 1;
                                    }
                                }
                                if (has)
                                {
                                    packet_cursor =
                                        menu_draw_clamped_number(ot_entry, packet_cursor, (u32)abs(total), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            break;
                            case 0x49:
                            {
                                void* base;
                                u8 idx;
                                draw_packet_cursor = packet_cursor;
                                base_a2_9 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x74));
                                base = (void*)g_pad_ctx;
                                {
                                    u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + 0x633;
                                    idx = *idxp;
                                }
                                {
                                    void* a2_2 = (void*)((u8*)base_a2_9 + *(u16*)((u8*)base_a2_9 + (idx * 2)));
                                    packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                  (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x4A:
                            {
                                s8 history_index = g_pad_ctx->large_history_index;
                                if (history_index >= 0)
                                {
                                    PadContext* context = g_pad_ctx;
                                    u8 v = menu_history_record(context, history_index)->unknown_0x44 & 0xF;
                                    void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x80));
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), 1,
                                                                  content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET, (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x4B:
                            {
                                s8 history_index = g_pad_ctx->large_history_index;
                                if (history_index >= 0)
                                {
                                    PadContext* context = g_pad_ctx;
                                    u8 v = menu_history_record(context, history_index)->unknown_0x44 >> 4;
                                    void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x7C));
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), 1,
                                                                  content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET, (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                }
                            }
                            break;
                            case 0x4C:
                            {
                                s8 history_index = g_pad_ctx->large_history_index;
                                if (history_index >= 0)
                                {
                                    PadContext* context = g_pad_ctx;
                                    u8 v = menu_history_record(context, history_index)->unknown_0x46;
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, v, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                            }
                            break;
                            case 0x4D:
                            {
                                s8 history_index = g_pad_ctx->large_history_index;
                                if (history_index >= 0)
                                {
                                    PadContext* context;
                                    s32 idx;
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_10 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x70));
                                    context = g_pad_ctx;
                                    idx = menu_history_record(context, history_index)->unknown_0x48;
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_10 + *(u16*)((u8*)base_a2_10 + (idx * 2)));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                            }
                            break;
                            case 0x4E:
                            case 0x4F:
                            case 0x50:
                            case 0x51:
                            case 0x52:
                            case 0x53:
                            case 0x54:
                            case 0x55:
                            {
                                u32 display_value = 0;
                                switch (content_index)
                                {
                                case 0x4E:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.packed & 0xF;
                                    break;
                                case 0x4F:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.bytes[0];
                                    display_value >>= 4;
                                    break;
                                case 0x50:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.packed >> 8;
                                    display_value &= 0xF;
                                    break;
                                case 0x51:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.packed >> 12;
                                    display_value &= 0xF;
                                    break;
                                case 0x52:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.halfwords[1];
                                    display_value &= 0xF;
                                    break;
                                case 0x53:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.packed >> 20;
                                    display_value &= 0xF;
                                    break;
                                case 0x54:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.bytes[3];
                                    display_value &= 0xF;
                                    break;
                                case 0x55:
                                    display_value = menu_equipped_item(g_pad_ctx, 0)->display_nibbles.packed >> 28;
                                    break;
                                }
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, display_value, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                            }
                            break;
                            default:
                                break;
                            }
                        }
                        break;

                        case 6:
                        {
                            shared_s0 = content_item->action_type;
                            switch (shared_s0)
                            {
                            case 1:
                                if (*(u32*)((void*)g_pad_ctx + 0x2C) > 0x989680U)
                                {
                                    packet_cursor = func_800A8A78(ot_entry, packet_cursor, 0x989680U, 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                else
                                {
                                    packet_cursor =
                                        func_800A8A78(ot_entry, packet_cursor, *(u32*)((void*)g_pad_ctx + 0x2C), 1, &pos, ((content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK));
                                }
                                break;
                            case 2:
                            {
                                u32 ltemp_v0_6;
                                s32 v1;
                                s32 s2;
                                s32 split_tmp;
                                s32 one;
                                ltemp_v0_6 = (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK;
                                switch (ltemp_v0_6)
                                {
                                case 1:
                                    pos.x -= 0x32;
                                    break;
                                case 2:
                                    pos.x -= 0x19;
                                    break;
                                }
                                v1 = *(s32*)((void*)g_pad_ctx + 0x30) + VSync(-1);
                                shared_s0 = v1 - D_80042FB4;
                                pos.x += 0x14;
                                s2 = shared_s0 / 216000;
                                one = 1;
                                packet_cursor = (void*)func_800A8A78(ot_entry, packet_cursor, s2, one, &pos, one);
                                if ((g_frame_counter / 15) & 1)
                                {
                                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, ":", one, pos.x, pos.y, 0);
                                }
                                pos.x += 7;
                                split_tmp = s2 * 0x3C;
                                shared_s0 = (shared_s0 / 3600) - split_tmp;
                                if (shared_s0 < 0xA)
                                {
                                    packet_cursor = (void*)func_800A8A78(ot_entry, packet_cursor, 0U, 1, &pos, 0);
                                }
                                pos.x += 0x10;
                                packet_cursor = (void*)func_800A8A78(ot_entry, packet_cursor, shared_s0, 1, &pos, one);
                            }
                            break;
                            }
                        }
                        break;

                        case 7:
                        {
                            shared_s0 = content_item->action_type;
                            switch (shared_s0)
                            {
                            case 1:
                            {
                                void* base = (void*)g_pad_ctx + g_menu_char_slot * 0x250;
                                if ((*(u8*)((u8*)base + 0x608) & 0x7F) != two_outer || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_11 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES]);
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_11 + *(u16*)((u8*)base_a2_11 + 0x74));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                            }
                            break;
                            case 2:
                            {
                                void* base = (void*)g_pad_ctx + g_menu_char_slot * 0x250;
                                if ((*(u8*)((u8*)base + 0x608) & 0x7F) != 2 || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                                {
                                    draw_packet_cursor = packet_cursor;
                                    base_a2_12 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES]);
                                    {
                                        void* a2_2 = (void*)((u8*)base_a2_12 + *(u16*)((u8*)base_a2_12 + 0x76));
                                        packet_cursor = func_800A88A0(draw_packet_cursor, ot_entry, a2_2, 1, content_item->packed_x & MENU_CONTENT_X_MASK, content_item->y - MENU_CONTENT_VIEW_Y_OFFSET,
                                                                      (content_item->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK);
                                    }
                                }
                            }
                            break;
                            }
                        }
                        break;

                        default:
                            break;
                        }
                    }

                    content_item++;
                    remaining_count--;
                } while (remaining_count != 0);
            }

            {
                MenuNode* nodes = g_menu_nodes;
                MenuNode* node = nodes + g_menu_scene_type;
                if (node->label_id == 0x13)
                {
                    u8* a2_27 = (u8*)g_menu_item_ptr;
                    if (*a2_27 != 0)
                    {
                        packet_cursor = func_800A88A0(packet_cursor, ot_entry, a2_27, 1, 0xAC, 0xC, 2);
                    }
                }
                else if (g_menu_scene_type == 0x1D)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES]);
                    u16 v1 = *(u16*)((u8*)a2_28 + 0x78);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, a2_27, 1, 0xAC, 0xC, 2);
                }
                else if (g_menu_scene_type != -1)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES]);
                    u16 v1 = *(u16*)((u8*)a2_28 + node->label_id * 2);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    packet_cursor = func_800A88A0(packet_cursor, ot_entry, a2_27, 1, 0xAC, 0xC, 2);
                }
            }

            switch (g_menu_scene_type)
            {
            case 20:
            case 21:
            case 23:
            case 24:
            case 26:
            case 27:
            {
                void* ltemp;
                pos.x = 0x88;
                pos.y = 0x28;
                if (g_menu_page_count != 0)
                {
                    ltemp = func_800AD208(ot_entry, packet_cursor, g_script_repeat_last + 1, 3, &pos, 0);
                }
                else
                {
                    ltemp = func_800AD208(ot_entry, packet_cursor, 0, 3, &pos, 0);
                }
                ltemp = func_800AD524(ltemp, ot_entry, 0xB, &pos, 0);
                pos.x += 8;
                ltemp = func_800AD208(ot_entry, ltemp, g_menu_page_count, 3, &pos, 0);
                ltemp = func_800AD524(ltemp, ot_entry, 0xB, &pos, 0);
                pos.x += 8;
                packet_cursor = func_800AD208(ot_entry, ltemp, menu_count_inventory_items(), 3, &pos, 0);
            }
            break;
            default:
                break;
            }
        }
    }

    return packet_cursor;
}


/**
 * @brief Build the equipped-item ability mask while excluding one equipment slot.
 * @param excluded_slot Equipment slot to omit from the mask.
 * @return Combined ability mask for the remaining active equipment entries.
 */
inline s32 menu_get_equipment_ability_mask(s32 excluded_slot)
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

/**
 * @brief Emit and link the menu texture-page command.
 * @param draw_mode Primitive-buffer cursor.
 * @param ot Ordering-table entry to prepend to.
 * @return Primitive-buffer cursor after the command.
 */
void* menu_emit_draw_mode_primitive(DR_TPAGE* draw_mode, s32* ot)
{
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot, draw_mode);
    draw_mode++;
    return draw_mode;
}

/**
 * @brief Concatenate two encoded text command streams into a destination buffer.
 * @param dst Buffer large enough for both streams and the terminator.
 * @param src1 First null-terminated stream.
 * @param src2 Second null-terminated stream.
 * @note Bytes 0x19-0x1F introduce a second byte, which may be zero.
 */
void menu_concat_encoded_text(u8* dst, u8* src1, u8* src2)
{
    menu_copy_encoded_pair(dst, src1, src2);
}


/**
 * @brief Extract a 4-bit nibble from a packed u32 field in the item struct and look it up in a byte table.
 * @param item Item record containing the packed nibble field.
 * @param index Nibble selector (0-7), selecting four bits at a time.
 * @param fallback Default table index used when @p index is out of range.
 * @return Signed byte from D_800F0C38 at the selected nibble index.
 */
s8 menu_lookup_item_nibble(const MenuItemEntry* item, u32 index, u32 fallback)
{
    u32 nibble;

    nibble = fallback;
    if (index < 8U)
    {
        switch (index)
        {
        case 0:
            nibble = item->nibbles.packed & 0xF;
            break;
        case 1:
            nibble = item->nibbles.bytes[0];
            nibble = nibble >> 4;
            break;
        case 2:
            nibble = item->nibbles.packed >> 8;
            nibble = nibble & 0xF;
            break;
        case 3:
            nibble = item->nibbles.packed >> 12;
            nibble = nibble & 0xF;
            break;
        case 4:
            nibble = item->nibbles.halfwords[1];
            nibble = nibble & 0xF;
            break;
        case 5:
            nibble = item->nibbles.packed >> 20;
            nibble = nibble & 0xF;
            break;
        case 6:
            nibble = item->nibbles.bytes[3];
            nibble = nibble & 0xF;
            break;
        case 7:
            nibble = item->nibbles.packed >> 28;
            break;
        }
    }
    return D_800F0C38[nibble];
}

/**
 * @brief Find the first active action or submenu item in the current scene.
 * @return Content-item index, or -1 if there is no scene or matching item.
 */
s32 menu_find_active_content_item(void)
{
    u8 content_id;
    s32 item_count;
    MenuContentItem* item;
    s32 item_index;
    u16 packed_x;
    u16 content_type;

    if (g_menu_scene_type == -1)
    {
        return -1;
    }

    content_id = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
    item = g_menu_content_table[content_id];
    item_count = g_menu_content_item_counts[g_menu_content_group_ids[content_id]];

    for (item_index = 0; item_index < item_count; item_index++, item++)
    {
        packed_x = item->packed_x;
        content_type = packed_x & MENU_CONTENT_ITEM_TYPE_MASK;
        if ((content_type == MENU_CONTENT_ITEM_TYPE_ACTION || content_type == MENU_CONTENT_ITEM_TYPE_SUBMENU) &&
            (packed_x & MENU_CONTENT_ITEM_ACTIVE_MASK) == MENU_CONTENT_ITEM_ACTIVE_MASK)
        {
            return item_index;
        }
    }

    return -1;
}

/**
 * @brief Find the navigation-list index of a given node ID.
 * @param node_id Node ID to locate.
 * @return Navigation index, or -1 if the node is absent.
 */
s32 menu_find_nav_node_index(s32 node_id)
{
    s32* nav_entry;
    s32 nav_index;
    s32 nav_count;

    nav_index = 0;
    if (g_menu_nav_count > 0)
    {
        nav_count = g_menu_nav_count;
        nav_entry = g_menu_nav_nodes;
        do
        {
            if (*nav_entry == node_id)
            {
                return nav_index;
            }
            nav_index++;
            nav_entry++;
        } while (nav_index < nav_count);
    }

    return -1;
}

/**
 * @brief Draw a slot's scroll arrows for content outside its viewport.
 * @param sprite Primitive write cursor.
 * @param ot_entry Ordering-table entry for the arrows.
 * @param slot Menu slot whose scroll fields drive the arrows.
 * @return Updated primitive write cursor.
 */
void* menu_emit_slot_scroll_arrows(SPRT* sprite, u_long* ot_entry, MenuSlot* slot)
{
    s32 arrow_count = 0;
    s32 remaining_height;
    u8* packet_cursor;

    if (slot->lerp_cur_b != 0)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        sprite->x0 = slot->x + slot->w - MENU_SCROLL_ARROW_SIZE;
        sprite->y0 = slot->y;
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_UP);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        arrow_count = 1;
        sprite++;
    }

    remaining_height = ((slot->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK) << 4) - slot->lerp_cur_b;
    if ((slot->h - MENU_SCROLL_ARROW_SIZE) < remaining_height)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        sprite->x0 = slot->x + slot->w - MENU_SCROLL_ARROW_SIZE;
        sprite->y0 = slot->y + slot->h - 8;
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_DOWN);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        addPrim(ot_entry, sprite);
        arrow_count += 1;
        sprite++;
    }

    packet_cursor = (u8*)sprite;
    if (arrow_count != 0)
    {
        DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot_entry, draw_mode);
        packet_cursor = (u8*)(draw_mode + 1);
    }

    return packet_cursor;
}

/**
 * @brief Draw navigation-tree scroll arrows for content outside the viewport.
 * @param sprite Primitive write cursor.
 * @param ot_entry Pointer to the ordering-table entry to prepend each arrow_count primitive to.
 * @return Pointer to the next free byte in @p sprite after all arrow_count primitives.
 */
void* menu_emit_tree_scroll_arrows(SPRT* sprite, s32* ot_entry)
{
    u8* packet_cursor;

    if (g_menu_content_height != 0)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, 0x20, 3);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_UP);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        sprite++;
    }

    if ((g_menu_layout_end - g_menu_content_height) > MENU_VIEW_HEIGHT)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, 0x20, 0xBA);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_DOWN);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        sprite++;
    }

    packet_cursor = (u8*)sprite;
    if ((g_menu_content_height != 0) || (g_menu_layout_end > MENU_VIEW_HEIGHT))
    {
        DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot_entry, draw_mode);
        packet_cursor = (u8*)(draw_mode + 1);
    }

    return packet_cursor;
}

/**
 * @brief Draw the active node's cursor, clamped to the visible viewport.
 * @param packet_cursor Primitive write cursor.
 * @param ot_entry Ordering-table entry for the arrow_count primitives.
 * @param draw_label Non-zero to also draw the node's label.
 * @return Updated primitive write cursor.
 */
s32 menu_draw_active_node_cursor(s32 packet_cursor, s32* ot_entry, s32 draw_label)
{
    u16 nav_x_packed;
    u8 nav_y_high;
    s32 nav_y_low;
    s32 node_y;
    s32 cursor_y;
    s32 viewport_y;
    u8* label_table;

    nav_x_packed = g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed;
    nav_y_high = g_menu_nodes[g_menu_active_node].u8_u.s.nav_y_hi;

    nav_y_low = nav_x_packed >> 15;
    node_y = (nav_y_high << 1) | nav_y_low;
    viewport_y = g_menu_content_height - MENU_CURSOR_Y_MIN;
    cursor_y = node_y - viewport_y;
    if (cursor_y < MENU_CURSOR_Y_MIN)
    {
        cursor_y = MENU_CURSOR_Y_MIN;
    }
    if (cursor_y >= MENU_CURSOR_Y_MAX)
    {
        cursor_y = MENU_CURSOR_Y_MAX;
    }

    packet_cursor = menu_emit_cursor(packet_cursor, ot_entry, menu_nav_x(nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET, cursor_y, 1);

    if (draw_label != 0)
    {
        label_table = menu_text_table_base(MENU_TEXT_GENERAL);
        packet_cursor = func_800A88A0(packet_cursor, ot_entry, menu_text_entry(label_table, g_menu_nodes[g_menu_active_node].label_id), 1, 0xA0, 0xCA, 2);
    }

    return packet_cursor;
}


/**
 * @brief Render the content cursor, update its lerped position, and optionally render the active hit-item label.
 * @param prim_buf Current primitive buffer pointer, reused as the running cursor.
 * @param ot Pointer to the current ordering-table entry.
 * @param draw_label Non-zero to also render the label string for the active hit item.
 * @return Updated primitive buffer pointer after all emitted primitives.
 */
void* menu_draw_content_cursor(void* prim_buf, s32* ot, s32 draw_label)
{
    u8 label_buffer[0x40];
    u8 item_name_buffer[0x40];
    s32* cursor_ot;
    s32 cursor_active;
    s16 text_index;
    MenuContentItem* content_base;
    u16 content_type;
    s32 action_type;
    DR_TPAGE* draw_mode;

    if (g_menu_content_ready == 0)
    {
        cursor_active = 0;
        if (menu_item_has_action() != 0)
        {
            cursor_active = (draw_label != 0);
        }
        cursor_ot = ot;
        if (g_menu_suppress_cursor != 0)
        {
            cursor_ot = (s32*)((u8*)ot - 0x28);
        }
        prim_buf = menu_emit_cursor(prim_buf, cursor_ot, g_content_cursor_x, g_content_cursor_y, cursor_active);
    }

    if (g_menu_suppress_cursor != 0)
    {
        s32* cxp = &g_content_cursor_x;
        s32 dx = (g_content_view_x - *cxp) / g_menu_suppress_cursor;
        s32* cyp = &g_content_cursor_y;
        s32 dy = (g_content_view_y - *cyp) / g_menu_suppress_cursor;
        g_menu_suppress_cursor -= 1;
        *cxp = dx + *cxp;
        *cyp = dy + *cyp;
    }
    else
    {
        g_content_cursor_x = g_content_view_x;
        g_content_cursor_y = g_content_view_y;
    }

    if (draw_label != 0)
    {
        content_base = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
        content_type = content_base[g_menu_hit_item_idx].packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

        if (content_type == MENU_CONTENT_ITEM_TYPE_ACTION)
        {
            if (content_base[g_menu_hit_item_idx].action_type < 0xF0U)
            {
                prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_GENERAL), content_base[g_menu_hit_item_idx].action_type));
            }
            else
            {
                action_type = content_base[g_menu_hit_item_idx].action_type;
                switch (action_type)
                {
                case 0xF0:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(17), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF1:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ABILITY_HELP), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF2:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(12), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF3:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(8), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF4:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_CATEGORY_ENTRY_HELP), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF5:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(21), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF6:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(19), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF7:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(23), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xF8:
                    if (D_80168C6C != 0xFF)
                    {
                        if (D_80168C6C & 0x80)
                        {
                            prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_CATEGORY_ENTRY_HELP), content_base[g_menu_hit_item_idx].action_type));
                        }
                        else
                        {
                            prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_TECHNIQUE_HELP), content_base[g_menu_hit_item_idx].action_type));
                        }
                    }
                    break;
                case 0xF9:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_TECHNIQUE_HELP), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xFA:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_SPELL_HELP), content_base[g_menu_hit_item_idx].action_type));
                    break;
                case 0xFB:
                case 0xFC:
                case 0xFD:
                case 0xFE:
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_KEY_ITEM_HELP), content_base[g_menu_hit_item_idx].action_type));
                }
            }
        }
        else if (content_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
        {
            action_type = content_base[g_menu_hit_item_idx].action_type;
            switch (action_type)
            {
            case 1:
            case 2:
            {
                u8* text_table = menu_text_table_base(MENU_TEXT_SPELL_HELP);
                u8* character_base = (u8*)g_pad_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                u8 entry_index = *(character_base + action_type + 0x609);

                prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                break;
            }
            case 3:
            case 4:
            case 5:
            case 6:
            {
                u8* pad_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8* flag_addr = pad_base + action_type;
                u8 flag = ((MenuNameSelection*)(flag_addr + 0x609))->packed;
                if (flag != MENU_NONE)
                {
                    if (flag & 0x80)
                    {
                        u8* item_ptr = pad_base + (((MenuNameSelection*)(flag_addr + 0x609))->fields.index << 6) + 0x740;
                        u8 cat = *(item_ptr + 0x24);
                        u8 entry = *(item_ptr + 0x25);
                        u8* state44 = (u8*)g_menu_state_ptr;
                        state44 += *(s32*)(state44 + 0x44);
                        prim_buf = menu_emit_content_label(prim_buf, ot, state44 + *(u16*)(state44 + (cat) * 0x1C + (entry) * 2));
                    }
                    else
                    {
                        u32 slot654 = *(u32*)(pad_base + 0x654);
                        u8* state1c = (u8*)g_menu_state_ptr;
                        state1c += *(s32*)(state1c + 0x1C);
                        prim_buf = menu_emit_content_label(prim_buf, ot, state1c + *(u16*)(state1c + ((MenuNameSelection*)(flag_addr + 0x609))->fields.index * 2 +
                                                        ((slot654 >> 0xA) & 0x3F) * 0x30));
                    }
                }
                break;
            }
            case 7:
            case 8:
            case 9:
            case 10:
                if (g_menu_char_slot < 2)
                {
                    PadContext* context = g_pad_ctx;
                    if (menu_equipped_item(context, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->active != 0)
                    {
                        if (menu_item_is_nondefault(&menu_character_record(context, g_menu_char_slot)->items[action_type - MENU_EQUIPMENT_SUBTYPE_BASE]) != 0)
                        {
                            s32 name_index = menu_equipped_item(g_pad_ctx, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->attributes.halves.high & 0x3F;
                            u16 name_offset =
                                *(u16*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_KEY_ITEM_NAMES] + name_index * 2);
                            u8* message_table = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES];
                            u16 suffix_offset = *(u16*)((u8*)message_table + 0xB4);
                            u8* name = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_KEY_ITEM_NAMES] + name_offset;
                            u8* suffix = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] + suffix_offset;
                            u8* out = item_name_buffer;
                            menu_copy_encoded_pair(out, name, suffix);
                        }
                        else
                        {
                            item_name_buffer[0] = 0;
                        }
                        {
                            PadContext* context = g_pad_ctx;
                            u32 item_attributes = menu_equipped_item(context, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->attributes.packed;
                            u32 kind = (item_attributes >> 8) & 3;
                            switch (kind)
                            {
                            case 0:
                            {
                                u32 idx = (item_attributes >> 9) & 0x7E;
                                u8* category_help_table = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP];
                                u8* str2 = category_help_table + *(u16*)((u8*)((s32)idx + (s32)category_help_table) + 0);

                                u8* out = label_buffer;
                                u8* first = item_name_buffer;
                                menu_copy_encoded_pair(out, first, str2);
                                break;
                            }
                            case 1:
                            {
                                u32 idx = (item_attributes >> 9) & 0x7E;
                                u8* category_help_table = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP];
                                u8* str2 = category_help_table + *(u16*)((u8*)((s32)idx + (s32)category_help_table) + 0x16);

                                u8* out = label_buffer;
                                u8* first = item_name_buffer;
                                menu_copy_encoded_pair(out, first, str2);
                                break;
                            }
                            default:
                            {
                                s32* category_help_offset = &((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP];
                                PadContext* context = g_pad_ctx;
                                u32 reloaded_item_attributes = menu_equipped_item(context, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->attributes.packed;
                                u32 idx = (reloaded_item_attributes >> 9) & 0x7E;
                                u8* category_help_table = (u8*)g_menu_state_ptr + *category_help_offset;
                                u8* str2 = category_help_table + *(u16*)((u8*)((s32)idx + (s32)category_help_table) + 0x2E);

                                u8* out = label_buffer;
                                u8* first = item_name_buffer;
                                menu_copy_encoded_pair(out, first, str2);
                                break;
                            }
                            }
                        }
                        prim_buf = menu_emit_content_label(prim_buf, ot, label_buffer);
                    }
                }
                break;
            case 11:
            case 12:
            case 13:
            {
                u8* char_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                if (*(char_base + 0x640) != 0)
                {
                    text_index = (s32) * (char_base + action_type + 0x65D);
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(23), text_index));
                }
                break;
            }
            case 14:
            {
                u8* text_table = menu_text_table_base(17);
                u8* character_base = (u8*)g_pad_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                u8 entry_index = character_base[0x609];

                prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                break;
            }
            case 15:
                if ((u8*)g_menu_item_ptr != NULL)
                {
                    prim_buf = menu_emit_content_label(prim_buf, ot, (u8*)g_menu_item_ptr);
                }
                break;
            case 16:
            case 17:
            case 18:
                if (g_menu_item_ptr != 0)
                {
                    text_index = (s32) * ((u8*)g_menu_item_ptr + action_type + 0x10);
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(21), text_index));
                }
                break;
            case 19:
            case 20:
            case 21:
                if (g_menu_item_ptr != 0)
                {
                    text_index = (s32) * ((u8*)g_menu_item_ptr + action_type + 0x15);
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(23), text_index));
                }
                break;
            case 22:
                if (g_menu_item_ptr != 0)
                {
                    u8* help_table = menu_text_table_base(MENU_TEXT_CATEGORY_ENTRY_HELP);
                    u8 cat = *((u8*)g_menu_category2_item + 0x24);
                    u8 entry = *((u8*)g_menu_category2_item + 0x25);
                    prim_buf = menu_emit_content_label(prim_buf, ot, help_table + *(u16*)(help_table + cat * 0x1C + entry * 2));
                }
                break;
            case 23:
                if (g_menu_item_ptr != 0)
                {
                    u8* text_table = menu_text_table_base(8);
                    u8* item = (u8*)g_menu_category2_item;
                    u8 entry_index = item[0x24];
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                }
                break;
            case 24:
                if (g_menu_item_ptr != 0)
                {
                    u8* text_table = menu_text_table_base(12);
                    u8* item = (u8*)g_menu_category2_item;
                    u8 entry_index = item[0x25];
                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                }
                break;
            case 25:
            {
                u8* text_table = menu_text_table_base(29);
                u8* character_base = (u8*)g_pad_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                u8 entry_index = character_base[0x633];

                prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                break;
            }

            }
        }
    }

    if (g_party_sort_marker.selected_idx != MENU_NONE)
    {
        prim_buf = menu_emit_sort_marker(prim_buf, ot, g_party_sort_marker.x, g_party_sort_marker.y);
        draw_mode = (DR_TPAGE*)prim_buf;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot, draw_mode);
        prim_buf = draw_mode + 1;
    }

    return prim_buf;
}


/* ----- Node Tree Rendering and Cursors ----- */

/**
 * @brief Render the root nodes and advance the tree's scroll animation.
 * @param prim_buf Current primitive buffer pointer.
 * @param ot Pointer to the ordering-table entry used by node-rendering helpers.
 * @return Updated primitive buffer pointer after rendering all active root nodes.
 */
void* menu_draw_node_tree(void* prim_buf, s32* ot)
{
    s32 node_index;
    s32 scroll_step;

    g_menu_nav_count = 0;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        if ((g_menu_nodes[node_index].u2.s.parent_idx == MENU_NONE) && (g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE))
        {
            prim_buf = menu_draw_node_recursive(node_index, prim_buf, ot);
        }
    }

    if ((g_menu_redraw_state == 0) || (g_menu_scroll_pos == g_menu_content_height))
    {
        g_menu_redraw_state = 0;
        g_menu_content_height = g_menu_scroll_pos;
    }
    else
    {
        scroll_step = (g_menu_scroll_pos - g_menu_content_height) / g_menu_redraw_state;
        g_menu_redraw_state -= 1;
        g_menu_content_height += scroll_step;
    }

    return prim_buf;
}

/**
 * @brief Render one menu node's panel and update its animated Y position; recurse into children.
 * @param node_index Node index within g_menu_nodes.
 * @param prim_buf Current primitive buffer pointer.
 * @param ot Pointer to the ordering-table entry.
 * @return Updated primitive buffer pointer after rendering this node and all expanded children.
 */
void* menu_draw_node_recursive(s32 node_index, void* prim_buf, s32* ot)
{
    MenuNode* node;
    MenuNode* node_base;
    int work_mask;
    int work_value;
    MenuNode* child_source;
    void* next_prim;
    int animation_count;
    int style_bits;

    style_bits = 3;
    g_menu_nav_nodes[g_menu_nav_count] = node_index;
    node_base = g_menu_nodes;
    node = node_base + node_index;
    g_menu_nav_count += 1;
    next_prim = menu_emit_icon_sprite(prim_buf, ot, node->icon_id, ((node->idx_nav.nav_x_packed >> 8) & (MENU_NAV_X_MASK >> 8)) + 1,
                                      ((node->u8_u.s.nav_y_hi << 1) | (work_value = node->idx_nav.nav_x_packed >> 15)) - g_menu_content_height, 1,
                                      ((node->u2.unk2 >> 2) & 3) != 0, g_menu_scene_type == node_index, (node->u2.unk2 >> 6) & style_bits);

    {
        u16 flags = node->u2.unk2;
        u32 animation_frames = (flags >> 2) & 3;
        if (animation_frames != 0)
        {
            s32 updated_flags;
            work_value = animation_frames - 1;
            animation_count = work_value & 3;
            work_mask = animation_count << 2;
            updated_flags = flags & 0xFFF3;
            node->u2.unk2 = updated_flags | work_mask;
        }
    }

    if (node->layout_frames_remaining == MENU_NODE_LAYOUT_IDLE)
    {
        s32 target_y_low_bit = node->u8_u.nav_y_packed & 0x8000;
        node->idx_nav.nav_x_packed = (node->idx_nav.nav_x_packed & 0x7FFF) | target_y_low_bit;
        {
            u16 packed_y = node->u8_u.nav_y_packed;
            packed_y &= 0xFF00;
            packed_y |= node->layout.s.layout_y_hi;
            node->u8_u.nav_y_packed = packed_y;
        }
    }
    else
    {
        u16 nav_x_packed = node->idx_nav.nav_x_packed;
        u16 nav_y_packed = node->u8_u.nav_y_packed;
        s32 current_y_low_bit = nav_x_packed >> 15;
        s32 current_y = ((nav_y_packed & 0xFF) << 1) | current_y_low_bit;
        u16 target_y = (nav_y_packed >> 15) | (node->layout.s.layout_y_hi << 1);

        if (((u16)current_y) == target_y)
        {
            node->layout_frames_remaining = MENU_NODE_LAYOUT_IDLE;
        }
        else
        {
            s32 step = ((s32)target_y - ((u16)current_y)) / ((s32)node->layout_frames_remaining);
            u32 new_y = current_y + step;
            new_y &= 0xFFFF;
            style_bits = 15;
            style_bits = (new_y & 1) << style_bits;
            ((volatile MenuNode*)node)->layout_frames_remaining -= 1;
            ((volatile MenuNode*)node)->idx_nav.nav_x_packed = (nav_x_packed & 0x7FFF) | style_bits;
            {
                u16 packed_y = ((volatile MenuNode*)node)->u8_u.nav_y_packed;
                packed_y &= 0xFF00;
                packed_y |= (new_y >> 1) & 0xFF;
                node->u8_u.nav_y_packed = packed_y;
            }
        }
    }

    {
        MenuNode* tree_node;
        MenuNode* tree_base = g_menu_nodes;
        tree_node = tree_base + node_index;
        if ((tree_node->u2.unk2 >> 1) & 1)
        {
            s32 child_slot = 0;
            s32 child_sentinel;
            child_source = tree_node;
            for (; child_slot < MENU_MAX_CHILDREN; child_slot++)
            {
                child_sentinel = MENU_NONE;
                if (child_source->layout.s.children[child_slot] == (u8)child_sentinel)
                {
                    break;
                }
                next_prim = menu_draw_node_recursive(child_source->layout.s.children[child_slot], next_prim, ot);
                child_sentinel = 0;
            }
        }
    }

    return next_prim;
}

/**
 * @brief Emit the sprite primitives for one menu icon.
 * @param prim_buf Primitive-buffer cursor.
 * @param ot Ordering-table entry to update.
 * @param icon_id Icon definition index.
 * @param x Base X coordinate.
 * @param y Base Y coordinate.
 * @param secondary_offset Nonzero to emit the secondary sprite and offset the primary sprite.
 * @param animation_offset Pixel offset applied to the animated sprite positions.
 * @param active Nonzero for an opaque secondary sprite; zero enables its semitransparency.
 * @param style_bits Packed node style bits; currently unused.
 * @return Next free primitive-buffer address.
 */
void* menu_emit_icon_sprite(void* prim_buf, s32* ot, s32 icon_id, s32 x, s32 y, s32 secondary_offset, s32 animation_offset, s32 active, s32 style_bits)
{
    SPRT* sprite = (SPRT*)prim_buf;
    s32 sprite_x;

    (void)style_bits;
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    sprite_x = x - secondary_offset;
    sprite->x0 = (s16)(sprite_x + animation_offset);
    sprite->y0 = (s16)((y - secondary_offset) + animation_offset);
    sprite->u0 = g_menu_icon_sprite_defs[icon_id].u_coord;
    sprite->v0 = g_menu_icon_sprite_defs[icon_id].v_coord;
    sprite->w = (s16)g_menu_icon_sprite_defs[icon_id].w;
    sprite->h = (s16)g_menu_icon_sprite_defs[icon_id].h;
    SET_SPRT_CLUT(sprite, menu_icon_clut(g_menu_icon_clut_codes[icon_id]));
    addPrim((u_long*)ot, sprite);
    sprite += 1;

    if (secondary_offset != 0)
    {
        if (active != 0)
        {
            SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0, 0, 0xA0));
        }
        else
        {
            SET_BGR0_PACKED(sprite, 0);
        }
        setSprt(sprite);
        if (active == 0)
        {
            setSemiTrans(sprite, 1);
        }
        sprite->x0 = (s16)(x + (secondary_offset - animation_offset) * 2);
        sprite->y0 = (s16)(y + (secondary_offset - animation_offset) * 2);
        sprite->u0 = g_menu_icon_sprite_defs[icon_id].u_coord;
        sprite->v0 = g_menu_icon_sprite_defs[icon_id].v_coord;
        sprite->w = (s16)g_menu_icon_sprite_defs[icon_id].w;
        sprite->h = (s16)g_menu_icon_sprite_defs[icon_id].h;
        SET_SPRT_CLUT(sprite, menu_icon_clut(g_menu_icon_clut_codes[icon_id]));
        addPrim((u_long*)ot, sprite);
        sprite += 1;
    }

    return sprite;
}

/**
 * @brief Emit a single 16x16 gray SPRT primitive and OT-link it.
 * @param prim_buf Current primitive buffer pointer; must have at least 0x14 bytes of space.
 * @param ot Pointer to the ordering-table entry; updated to prepend this SPRT.
 * @param x X screen position of the sprite.
 * @param y Y screen position of the sprite.
 * @return Pointer to the next free byte after the emitted 0x14-byte SPRT.
 */
void* menu_emit_sort_marker(void* prim_buf, s32* ot, s16 x, s16 y)
{
    SPRT* sprite = (SPRT*)prim_buf;

    SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0x50, 0x50, 0x50));
    setSprt(sprite);
    SET_SPRT_WH_PACKED(sprite, 16, 16);
    SET_SPRT_UV0_PACKED(sprite, 0x80);
    sprite->x0 = x;
    sprite->y0 = y;
    SET_SPRT_CLUT(sprite, getClut(0x60, MENU_ICON_CLUT_Y_BASE));
    addPrim((u_long*)ot, sprite);
    return sprite + 1;
}
