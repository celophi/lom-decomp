#include "internal/menu_internal.h"
#include "overlays/field/field_text.h"
#include "main/audio/akao.h"
#include "main/display.h"

s32 menu_find_active_content_item(void);
s32 menu_build_special_technique_nav_entries(void);

/**
 * @brief Handle list navigation and draw its cursor.
 * @param prim_buf Primitive-buffer cursor.
 * @param ot Ordering-table entry for the cursor.
 * @param state List state to update.
 * @param entries Packed row positions and circular navigation links.
 * @param view_origin Viewport origin in list coordinates.
 * @param active Nonzero to process input and animate the cursor.
 * @return Primitive-buffer cursor after drawing.
 */
void* scroll_list_draw(void* prim_buf, u_long* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active)
{
    int steps_remaining;
    if (active)
    {
        if (g_pad_input & PADR1)
        {
            g_pad_input = PADLdown;
            steps_remaining = (state->viewport_h - MENU_ITEM_NAV_POSITION_STRIDE) >> 4;
        }
        else if (g_pad_input & PADL1)
        {
            g_pad_input = PADLup;
            steps_remaining = (state->viewport_h - MENU_ITEM_NAV_POSITION_STRIDE) >> 4;
        }
        else
        {
            steps_remaining = 1;
        }
        while (steps_remaining != 0)
        {
            if (g_pad_input & MENU_PAD_VERTICAL)
            {
                if (g_pad_input & PADLup)
                {
                    state->navigation.fields.selected_index =
                        (entries[state->navigation.fields.selected_index] >> MENU_ITEM_NAV_PREVIOUS_SHIFT) & MENU_ITEM_NAV_INDEX_MASK;
                }
                else
                {
                    state->navigation.fields.selected_index = entries[state->navigation.fields.selected_index] >> MENU_ITEM_NAV_NEXT_SHIFT;
                }
                scroll_list_update_target(state, entries);
                if (state->navigation.fields.selected_index == ((state->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK) - 1))
                {
                    steps_remaining = 1;
                }
                if (state->navigation.fields.selected_index == 0)
                {
                    steps_remaining = 1;
                }
            }
            steps_remaining--;
        }

        if (g_pad_input & MENU_PAD_VERTICAL)
        {
            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        }
        if (g_pad_input & PADLleft)
        {
            g_pad_input |= PAD_BTN_CIRCLE;
        }
    }
    prim_buf = menu_emit_cursor(prim_buf, ot, (4 - view_origin->x) - state->scroll_x,
                                ((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - view_origin->y) - state->scroll_y, active);
    g_menu_default_view_pos.x = (state->base_x + ((4 - (u32)view_origin->x) - state->scroll_x)) + 8;
    g_menu_default_view_pos.y =
        (state->base_y + (((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - view_origin->y) - state->scroll_y)) + 8;
    return prim_buf;
}

/**
 * @brief Set scroll targets to keep the selected row visible.
 * @param state List state to update.
 * @param entries Packed row positions and circular navigation links.
 */
void scroll_list_update_target(ScrollListState* state, u32* entries)
{
    s32 item_y;

    if (4 - state->scroll_x > state->viewport_w - 32)
    {
        state->target_x = 4 - 32 - state->viewport_w;
    }
    if (4 - state->scroll_x < 0)
    {
        state->target_x = 4;
    }

    if ((s32)((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - state->scroll_y) > state->viewport_h - 32)
    {
        state->target_y = (entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - state->viewport_h + 32;
    }

    item_y = entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK;
    if (item_y - state->scroll_y < 0)
    {
        state->target_y = item_y;
    }
    state->lerp_steps = 4;
}

/**
 * @brief Draw the menu cursor with an optional animated shadow.
 * @param prim_buf Primitive write cursor; the SPRTs are built here.
 * @param ot Ordering-table entry; updated after each emitted primitive.
 * @param x Cursor screen X before the bob offset is applied.
 * @param y Cursor screen Y before the bob offset is applied.
 * @param active Nonzero to animate the cursor and draw its semitransparent shadow.
 * @return Primitive-buffer cursor after the sprites and draw-mode packet.
 */
void* menu_emit_cursor(void* prim_buf, u_long* ot, s32 x, s32 y, s32 active)
{
    SPRT* sprite = prim_buf;
    DR_TPAGE* draw_mode;
    u8* icon_id;
    MenuIconSpriteInfo* sprite_defs;
    u8* clut_codes;
    s32 bob_phase;

    if (active == 0 || g_menu_frame < 9)
    {
        bob_phase = 0;
    }
    else
    {
        if (g_menu_frame < 16)
        {
            bob_phase = 1;
        }
        else if (g_menu_frame < 23)
        {
            bob_phase = 2;
        }
        else if (g_menu_frame < 30)
        {
            bob_phase = 1;
        }
        else
        {
            bob_phase = 0;
            g_menu_frame = 0;
        }
    }

    sprite_defs = g_menu_icon_sprite_defs;
    icon_id = &g_menu_cursor_icon_ids[bob_phase];
    clut_codes = g_menu_icon_clut_codes;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x - bob_phase, y + bob_phase);
    setUV0(sprite, sprite_defs[*icon_id].u_coord, sprite_defs[*icon_id].v_coord);
    setWH(sprite, sprite_defs[*icon_id].w, sprite_defs[*icon_id].h);
    SET_SPRT_CLUT(sprite, menu_icon_clut(clut_codes[*icon_id]));
    addPrim(ot, sprite);
    sprite++;

    if (active != 0)
    {
        SET_BGR0_PACKED(sprite, 0);
        setSprt(sprite);
        setSemiTrans(sprite, 1);
        setXY0(sprite, (x - bob_phase) + 2, (y + bob_phase) + 2);
        setUV0(sprite, sprite_defs[*icon_id].u_coord, sprite_defs[*icon_id].v_coord);
        setWH(sprite, sprite_defs[*icon_id].w, sprite_defs[*icon_id].h);
        SET_SPRT_CLUT(sprite, menu_icon_clut(clut_codes[*icon_id]));
        addPrim(ot, sprite);
        sprite++;
    }

    draw_mode = (DR_TPAGE*)sprite;
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw the current inventory category and handle item selection.
 * @param ot Ordering table the primitives are linked into.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input this frame.
 * @return The advanced primitive write cursor.
 */

void* menu_inventory_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, s32 active)
{
    ScrollListState* list;
    void* packet_cursor;
    s32 scroll_y;
    void* pending_item_record;
    s32 item_index;
    s32 y;
    MenuItemEntry* item_record;

    MenuSlotRect rect;
    u8 item_name_buffer[0x40];

    list = state;
    packet_cursor = prim_buf;

    if ((g_pad_input & 0x10) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        switch (g_menu_active_item_category)
        {
        case 0:
        {
            s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x21:
            case 0x22:
            case 0x23:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                break;
            case 0x13:
            case 0x24:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x21;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_8016869F[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        case 1:
        {
            s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x25:
            case 0x26:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                break;
            case 0x16:
            case 0x27:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x25;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_8016869F[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        case 2:
        {
            u8 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x28:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x29;
                break;
            case 0x29:
            case 0x19:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x28;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_801686A0[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        }
    }

    packet_cursor = scroll_list_draw(packet_cursor, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if (g_menu_scene_type < 0x13 && (g_pad_input & 0x8000))
    {
        g_pad_input &= ~0x40;
    }

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        if (g_menu_scene_type < 0x13)
        {
            s32 item_kind;
            g_menu_content_ready = 0;
            item_kind = g_menu_active_item_category;
            if (item_kind == 2)
            {
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = (s8)item_kind;
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = (u8)((g_menu_char_slot * 3) + 2);
                menu_clear_slots();
                return packet_cursor;
            }
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 1;
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = (u8)((g_menu_char_slot * 3) + 3);
            menu_clear_slots();
            return packet_cursor;
        }
        if (g_menu_pending_item_row != MENU_NONE)
        {
            g_menu_pending_item_row = MENU_NONE;
        }
        else
        {
            g_menu_nodes[0x13].label_id = 0x11;
            g_menu_nodes[0x16].label_id = 0x14;
            g_menu_nodes[0x19].label_id = 0x16;
            g_menu_content_ready = 0;
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
            menu_reset_content_view();
            g_pad_input = 0;
        }
    }

    item_index = 0;
    y = item_index;
    scroll_y = list->scroll_y;
    g_menu_item_ptr = 0;
    g_menu_category0_item = 0;
    g_menu_category1_item = 0;
    g_menu_category2_item = 0;
    item_record = (MenuItemEntry*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET);

    while (item_index < MENU_ITEM_TABLE_COUNT)
    {
        u32 item_attributes;
        s32 item_kind;
        s32 icon_id;

        if (item_record->active == 0)
        {
            break;
        }
        item_attributes = item_record->attributes.packed;
        item_kind = FIELD_ITEM_CATEGORY(item_attributes);
        icon_id = 0x45;

        if (item_kind == g_menu_active_item_category)
        {
            switch (item_kind)
            {
            case 0:
                icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x45;
                break;
            case 1:
                icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x50;
                break;
            case 2:
                icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x5C;
                break;
            }

            if ((y - scroll_y) >= -0xF && (y - scroll_y) < (list->viewport_h - 0x10))
            {
                u32 display_attributes;
                s32 text_palette;
                u8 ability_flags;

                packet_cursor = menu_emit_icon_sprite(packet_cursor, ot, icon_id, 0x10 - view_origin->x, (y - scroll_y) - view_origin->y, 0, 0, 0, 0);
                setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
                addPrim(ot, (DR_TPAGE*)packet_cursor);
                packet_cursor = (DR_TPAGE*)packet_cursor + 1;

                display_attributes = item_record->attributes.packed;
                if (display_attributes & FIELD_ITEM_CATEGORY_BITS)
                {
                    ability_flags = g_field_armor_type_conflicts[FIELD_ITEM_TYPE(display_attributes)];
                }
                else
                {
                    ability_flags = g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(display_attributes)];
                }
                text_palette = 1;
                if (ability_flags & (u8)g_menu_equipment_conflicts)
                {
                    text_palette = 3;
                }
                packet_cursor = field_draw_text(packet_cursor, ot, (u8*)item_record, text_palette, 0x22 - view_origin->x, (y - scroll_y) - view_origin->y,
                                                FIELD_TEXT_ALIGN_LEFT);

                if (g_menu_pending_item_row != MENU_NONE)
                {
                    if ((y >> 4) == g_menu_pending_item_row)
                    {
                        s32 view_x = view_origin->x;
                        s32 view_y = view_origin->y;
                        SET_BGR0_PACKED((SPRT*)packet_cursor, GPU_COLOR_WORD(0x50, 0x50, 0x50));
                        setSprt((SPRT*)packet_cursor);
                        SET_SPRT_UV0_PACKED((SPRT*)packet_cursor, 0x80);
                        SET_SPRT_CLUT((SPRT*)packet_cursor, getClut(0x60, VRAM_CLUT_Y));
                        SET_SPRT_WH_PACKED((SPRT*)packet_cursor, 16, 16);
                        SET_YX0((SPRT*)packet_cursor, (y - scroll_y) - view_y, -2 - view_x);
                        addPrim(ot, (SPRT*)packet_cursor);
                        packet_cursor = (SPRT*)packet_cursor + 1;
                        setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
                        addPrim(ot, (DR_TPAGE*)packet_cursor);
                        packet_cursor = (DR_TPAGE*)packet_cursor + 1;
                    }
                }
            }

            if (g_menu_pending_item_row != MENU_NONE && (y >> 4) == g_menu_pending_item_row)
            {
                pending_item_record = &((MenuItemEntry*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET))[item_index];
            }

            if ((y >> 4) == list->navigation.fields.selected_index)
            {
                g_menu_inventory_index = item_index;
                g_menu_item_ptr = &((MenuItemEntry*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET))[item_index];
                switch (g_menu_active_item_category)
                {
                case 0:
                    g_menu_category0_item = g_menu_item_ptr;
                    break;
                case 1:
                    g_menu_category1_item = g_menu_item_ptr;
                    break;
                case 2:
                    g_menu_category2_item = g_menu_item_ptr;
                    break;
                }
            }
            y += 0x10;
        }

        item_index += 1;
        item_record += 1;
    }

    if ((y - scroll_y) <= 0 && list->lerp_steps == 0)
    {
        u16 cur = list->scroll_y;
        if (cur != 0)
        {
            list->target_y = cur - 0x10;
            list->lerp_steps = 4;
        }
    }

    list->navigation.packed = (list->navigation.packed & ~MENU_LIST_COUNT_MASK) |
                              ((menu_build_inventory_nav_entries(g_menu_active_item_category) & MENU_ITEM_NAV_INDEX_MASK) << MENU_LIST_COUNT_SHIFT);
    g_menu_page_count = (list->navigation.packed >> MENU_LIST_COUNT_SHIFT) & MENU_ITEM_NAV_INDEX_MASK;
    g_script_repeat_last = list->navigation.fields.selected_index;

    if (list->navigation.packed & MENU_LIST_COUNT_MASK)
    {
        if ((u32)((list->navigation.packed >> MENU_LIST_COUNT_SHIFT) & MENU_ITEM_NAV_INDEX_MASK) <= (u32)list->navigation.fields.selected_index)
        {
            menu_step_item_selection(-1);
            list->navigation.fields.selected_index = (u16)((list->navigation.fields.count_and_ot & 0x1FF) - 1);
            if (((list->navigation.fields.selected_index * 0x10) - scroll_y) < (list->viewport_h - 0x10))
            {
                u16 cur = list->scroll_y;
                if (cur != 0)
                {
                    list->target_y = cur - 0x10;
                    list->lerp_steps = 4;
                }
            }
        }
        g_menu_page_count = list->navigation.fields.count_and_ot & 0x1FF;
        g_script_repeat_last = list->navigation.fields.selected_index;

        if (g_menu_item_ptr != 0)
        {
            if (active != 0)
            {
                if (menu_item_differs_from_bare_hands(g_menu_item_ptr) != 0)
                {
                    u8* resources = g_menu_state_ptr;
                    u16 name_attributes = g_menu_item_ptr->attributes.halves.high;
                    s32 name_offset = (name_attributes & 0x3F) * 2;
                    s32 names_offset = ((MenuTextResources*)resources)->table_offsets[MENU_TEXT_ITEM_NAMES];
                    s32 general_offset = ((MenuTextResources*)resources)->table_offsets[MENU_TEXT_GENERAL];
                    uintptr_t name_table = (uintptr_t)resources + names_offset;
                    u8* name = (u8*)(name_table + *(u16*)(name_offset + name_table));
                    u8* general_table = resources + general_offset;
                    u8* suffix = menu_text_entry(general_table, 109);
                    u8* text_cursor = item_name_buffer;
                    menu_copy_encoded_pair(text_cursor, name, suffix);
                }
                else
                {
                    item_name_buffer[0] = 0;
                }

                {
                    u32 item_attributes = g_menu_item_ptr->attributes.packed;
                    u32 item_kind = FIELD_ITEM_CATEGORY(item_attributes);
                    switch (item_kind)
                    {
                    case 0:
                    {
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)menu_text_table_base(MENU_TEXT_ITEM_CATEGORY_HELP);
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                    case 1:
                    {
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)menu_text_table_base(MENU_TEXT_ITEM_CATEGORY_HELP);
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table + 11 * sizeof(u16)));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                    default:
                    {
                        s32* description_offset = &((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_CATEGORY_HELP];
                        u32 item_attributes = g_menu_item_ptr->attributes.packed;
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)g_menu_state_ptr + *description_offset;
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table + 23 * sizeof(u16)));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                    }
                    g_menu_help_text = (u8*)g_menu_item_description_buffer;
                }
            }
        }

        if (g_pad_input & PAD_CONFIRM_BUTTONS)
        {
            if (active != 0)
            {
                if (g_menu_scene_type < 0x13)
                {
                    s32 sub = g_menu_active_item_category;

                    if (sub == 2)
                    {
                        menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                        field_compact_inventory();
                        {
                            s32 char_slot = g_menu_char_slot;
                            u8* ctx = (u8*)g_saved_game_ctx + (char_slot * 0x250);
                            ctx += g_menu_active_subtype;
                            ctx[MENU_SKILL_OFFSET - MENU_SKILL_SUBTYPE_BASE] = (s8)((u8)g_menu_active_subtype + (FIELD_SKILL_INSTRUMENT - MENU_SKILL_SUBTYPE_BASE));
                        }
                        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = (s8)sub;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = (u8)((g_menu_char_slot * 3) + 2);
                        menu_clear_slots();
                        g_menu_content_ready = 0;
                    }
                    else
                    {
                        u32 w = g_menu_item_ptr->attributes.packed;
                        u8 ability_flags;
                        if (w & FIELD_ITEM_CATEGORY_BITS)
                        {
                            ability_flags = g_field_armor_type_conflicts[FIELD_ITEM_TYPE(w)];
                        }
                        else
                        {
                            ability_flags = g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(w)];
                        }
                        if (ability_flags & (u8)g_menu_equipment_conflicts)
                        {
                            menu_play_se(SFX_ERROR, AKAO_SFX_DEFAULT_VOLUME);
                            return packet_cursor;
                        }
                        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
                        menu_clear_slots();
                        if (menu_item_differs_from_bare_hands(g_menu_active_equipped_item) != 0)
                        {
                            menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = g_menu_item_ptr;
                            g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
                        }
                        else
                        {
                            field_copy_inventory_record(g_menu_active_equipped_item, g_menu_item_ptr);
                            g_menu_item_ptr->active = 0;
                            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 0;
                            g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
                        }
                        menu_clear_slots();
                        rect.x = 0xB0;
                        rect.y = 0x40;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = &menu_equipment_compare_callback;
                        g_menu_compare_window_active = 1;
                        ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);
                        menu_relink_pair();
                        g_menu_content_ready = 0;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 1;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = (u8)((g_menu_char_slot * 3) + 3);
                        return packet_cursor;
                    }
                }
                else
                {
                    u16 sel;

                    g_menu_nodes[0x13].label_id = 0x11;
                    g_menu_nodes[0x16].label_id = 0x14;
                    g_menu_nodes[0x19].label_id = 0x16;
                    menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                    sel = list->navigation.fields.selected_index;
                    if (sel != g_menu_pending_item_row)
                    {
                        if (g_menu_pending_item_row != MENU_NONE)
                        {
                            menu_swap_item_records(pending_item_record, g_menu_item_ptr);
                            g_menu_pending_item_row = MENU_NONE;
                        }
                        else
                        {
                            g_menu_pending_item_row = sel;
                        }
                    }
                    else
                    {
                        rect.x = 0xB0;
                        rect.y = 0x60;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = &menu_item_followup_callback;
                        ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);
                        menu_relink_pair();
                    }
                }
            }
        }
    }
    return packet_cursor;
}

/**
 * @brief Clear every skill slot of the active character that holds a technique; instruments stay.
 * @return 1 if at least one technique was removed, 0 if none were.
 */
s32 menu_remove_technique_skills(void)
{
    s32 changed;
    s32 i;

    changed = 0;
    for (i = 0; i < FIELD_SKILL_SLOT_COUNT; i++)
    {
        u8* skill = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + i;

        if (skill[MENU_SKILL_OFFSET] != FIELD_SKILL_NONE)
        {
            if ((skill[MENU_SKILL_OFFSET] & FIELD_SKILL_INSTRUMENT) == 0)
            {
                skill[MENU_SKILL_OFFSET] = FIELD_SKILL_NONE;
                changed = 1;
            }
        }
    }
    return changed;
}

/**
 * @brief Scan the item table for the next entry whose kind matches g_menu_active_item_category.
 * @param step Direction/stride to walk the table (1 = forward, -1 = backward).
 * @return Selected item-table index.
 */
u32 menu_step_item_selection(s32 step)
{
    u32 start;
    u32 index;
    s32 kind;
    MenuItemEntry* item;
    u32 result;
    u8* base;

    g_menu_item_ptr = 0;
    base = (u8*)g_saved_game_ctx;
    start = g_menu_inventory_index + step;
    item = (MenuItemEntry*)(base + ((start << 6) + MENU_ITEM_TABLE_OFFSET));
    index = start;
    while (index < MENU_ITEM_TABLE_COUNT)
    {
        if (item->active != 0)
        {
            kind = FIELD_ITEM_CATEGORY(item->attributes.packed);
            if (kind == g_menu_active_item_category)
            {
                g_menu_inventory_index = index;
                g_menu_item_ptr = (MenuItemEntry*)((u8*)g_saved_game_ctx + ((index << 6) + MENU_ITEM_TABLE_OFFSET));
                switch (kind)
                {
                case 0:
                    g_menu_category0_item = g_menu_item_ptr;
                    break;
                case 1:
                    g_menu_category1_item = g_menu_item_ptr;
                    break;
                case 2:
                    g_menu_category2_item = g_menu_item_ptr;
                    break;
                }
                return index;
            }
        }
        index += step;
        item += step;
    }

    result = (uintptr_t)g_menu_item_ptr;
    if (result == 0)
    {
        if (step == 1)
        {
            g_menu_inventory_index = -1;
        }
        else
        {
            g_menu_inventory_index = 0x64;
        }
        result = menu_step_item_selection(step);
    }
    return result;
}

/**
 * @brief Draw the character's spell/ability grid and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this grid.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_spell_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    s32 selected_index;
    s32 row;
    s32 col;
    s32 bit_mask;
    s32 y;
    s32 relative_y;
    u8* presence_row;
    u32 scroll_y;
    void* name_table;
    void* help_table;
    ScrollListState* list;

    list = state;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
        g_pad_input = 0;
    }

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    y = 0;
    selected_index = -1;
    row = 0;
    presence_row = (u8*)g_saved_game_ctx + MENU_SPELL_GRID_OFFSET;
    scroll_y = list->scroll_y;

    while (row < MENU_SPELL_GRID_ROW_COUNT)
    {
        col = 0;
        bit_mask = 1;
        while (col < MENU_SPELL_GRID_COLUMN_COUNT)
        {
            if (*presence_row & bit_mask)
            {
                relative_y = y - scroll_y;
                if ((relative_y >= -0xF) && (relative_y < (list->viewport_h - 0x10)))
                {
                    name_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_SPELL_NAMES]);
                    prim_buf = field_draw_text(prim_buf, ot, (void*)((u8*)name_table + *(u16*)((u8*)name_table + (col * 2) + (row * 0x10))), 1,
                                               0x10 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                }
                if (list->navigation.fields.selected_index == (y >> 4))
                {
                    selected_index = col + (row * 8);
                }
                y += 0x10;
            }
            col += 1;
            bit_mask <<= 1;
        }
        row += 1;
        presence_row += 1;
    }

    if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        *((u8*)g_saved_game_ctx + (g_menu_char_slot * 0x250) + g_menu_active_subtype + (row = 0x609)) = selected_index;
        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }

    if (selected_index != -1)
    {
        help_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_SPELL_HELP]);
        g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
    }

    return prim_buf;
}

/**
 * @brief Draw a character's equipment grid and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this grid.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_event_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    ScrollListState* list;
    u32 scroll_y;
    s32 selected_index;
    s32 row;
    s32 col;
    s32 y;
    s32 relative_y;
    s32 entry_kind;
    u32 row_entries;
    void* name_table;
    void* help_table;
    DR_TPAGE* draw_mode;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    scroll_y = list->scroll_y;
    selected_index = -1;

    for (row = 0; row < MENU_EVENT_WORD_COUNT; row += 1)
    {
        row_entries = *(u32*)((u8*)g_saved_game_ctx + (row << 2) + OFFSETOF(SavedGameLayout, words[SAVED_GAME_WORD_EVENT_PROGRESS]));
        for (col = 0; col < MENU_EVENTS_PER_WORD; col += 1)
        {
            entry_kind = row_entries & MENU_EVENT_STAGE_MASK;
            if (entry_kind >= MENU_EVENT_STAGE_LISTED)
            {
                relative_y = y - scroll_y;
                if ((relative_y >= -0xF) && (relative_y < (list->viewport_h - 0x10)))
                {
                    name_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_EVENT_NAMES]);
                    prim_buf = field_draw_text(prim_buf, ot, (void*)((u8*)name_table + *(u16*)((u8*)name_table + (col * 2) + (row * 0x10))), 1,
                                               0x20 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                    if (entry_kind >= 8)
                    {
                        if (entry_kind >= 0xF)
                        {
                            prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x70, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);
                        }
                        else
                        {
                            prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x69, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);
                        }

                        draw_mode = prim_buf;
                        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
                        addPrim(ot, draw_mode);
                        prim_buf = draw_mode + 1;
                    }
                }
                if (list->navigation.fields.selected_index == (y >> 4))
                {
                    selected_index = col + (row * 8);
                }
                y += 0x10;
            }
            row_entries >>= 4;
        }
    }

    if ((active != 0) && (selected_index != -1))
    {
        help_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_EVENT_HELP]);
        g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
    }

    return prim_buf;
}

/**
 * @brief Draw the Items list (held item kinds and their quantities) and handle its input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y) and the quantity anchor is (0xC0 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_item_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    s32 item_index;
    s32 y;
    s32 relative_y;
    u32 scroll_y;
    u8* quantity;
    s32 selected_index;
    void* name_table;
    void* help_table;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    selected_index = -1;
    quantity = g_saved_game_ctx->item_counts;
    scroll_y = list->scroll_y;

    for (item_index = 0; item_index < FIELD_ITEM_KIND_COUNT; item_index += 1)
    {
        if (*quantity != 0)
        {
            relative_y = y - scroll_y;
            if ((relative_y >= -0xF) && (relative_y < (list->viewport_h - 0x10)))
            {
                pos.x = 0xC0 - (u16)view_origin->x;
                pos.y = relative_y - (u16)view_origin->y;

                name_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_NAMES]);
                prim_buf = field_draw_text(prim_buf, ot, ((u8*)name_table + ((u16*)name_table)[item_index]), 1, 0x10 - view_origin->x,
                                           relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                prim_buf = menu_draw_clamped_number(ot, prim_buf, *quantity, 1, &pos, FIELD_TEXT_ALIGN_RIGHT);
            }
            if (list->navigation.fields.selected_index == (y >> 4))
            {
                selected_index = item_index;
            }
            y += 0x10;
        }
        quantity += 1;
    }

    if (selected_index != -1)
    {
        if (active != 0)
        {
            help_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_HELP]);
            g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
        }
    }

    return prim_buf;
}

/**
 * @brief Draw the Artifacts list (one entry per placed land) and handle its input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y) and the marker origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_artifact_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    u32 scroll_y;
    s32 selected_index;
    s32 land_index;
    s32 list_y;
    s32 relative_y;
    FieldLandRecord* land;
    void* name_table;
    void* help_table;
    DR_TPAGE* draw_mode;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    list_y = 0;
    selected_index = -1;
    land = g_saved_game_ctx->lands;
    scroll_y = list->scroll_y;

    for (land_index = 0; land_index < FIELD_LAND_COUNT; land_index += 1)
    {
        if (land->flags & FIELD_LAND_PLACED)
        {
            relative_y = list_y - scroll_y;
            if ((relative_y >= -0xF) && (relative_y < (list->viewport_h - 0x10)))
            {
                name_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ARTIFACT_NAMES]);
                prim_buf = field_draw_text(prim_buf, ot, (void*)((u8*)name_table + ((u16*)name_table)[land_index]), 1, 0x20 - view_origin->x,
                                           relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                if (land->flags & FIELD_LAND_FLAG_02)
                {
                    prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x2C, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);

                    draw_mode = prim_buf;
                    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
                    addPrim(ot, draw_mode);
                    prim_buf = draw_mode + 1;
                }
            }
            if (list->navigation.fields.selected_index == (list_y >> 4))
            {
                selected_index = land_index;
            }
            list_y += 0x10;
        }
        land += 1;
    }

    if (selected_index != -1)
    {
        if (active != 0)
        {
            help_table = (void*)(g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ARTIFACT_HELP]);
            g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
        }
    }

    return prim_buf;
}

/**
 * @brief Handle input for the equip/status page and draw its four state-table labels.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor (unchanged on the early-return paths).
 */
void* menu_subtype_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    s32 packed;
    s32 hi;
    MenuItemEntry* handle;
    s32 off;
    s32 i;
    s32 i1;
    s32 i3;
    u8 flag;
    MenuContentItem* items;
    ScrollListState* list;
    void* buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        switch (list->navigation.fields.selected_index)
        {
        case 0:
            list->anim_frame = 0;
            list->active = 0;
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            list = (ScrollListState*)menu_slot_alloc(3, &rect);
            ((MenuSlot*)list)->content_cb = &menu_special_technique_list_callback;
            packed = menu_build_special_technique_nav_entries();
            hi = packed >> 0x10;
            ((MenuSlot*)list)->lerp_target_b = hi * 0x10;
            ((MenuSlot*)list)->lerp_cur_b = hi * 0x10;
            ((MenuSlot*)list)->anim_frame = MENU_WINDOW_OPEN_FRAME;
            ((MenuSlot*)list)->active = MENU_SLOT_STATE_OPEN;
            ((MenuSlot*)list)->has_title = 1;
            g_menu_draw_early_out = 1;
            ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, packed);
            ((MenuSlot*)list)->navigation.fields.selected_index = (u16)hi;
            return buf;

        case 1:
            i1 = MENU_SLOT_COUNT - 1;
            while (i1 >= 0)
            {
                g_menu_slots[i1].active = 0;
                i1--;
            }
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = 0x29;
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = D_801686A0[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx];
            g_menu_equipment_conflicts = 0;
            g_menu_content_ready = 1;
            menu_open_content_page(MENU_PAGE_INSTRUMENTS);
            g_menu_draw_early_out = 1;
            g_menu_item_ptr = 0;
            g_menu_category0_item = 0;
            g_menu_category1_item = 0;
            g_menu_category2_item = 0;
            return buf;

        case 2:
        {
            u8* flag_ptr = (u8*)g_saved_game_ctx + (g_menu_char_slot * 0x250);
            flag_ptr += g_menu_active_subtype;
            flag = *(flag_ptr + (MENU_SKILL_OFFSET - MENU_SKILL_SUBTYPE_BASE));
            if (flag == FIELD_SKILL_NONE)
            {
                break;
            }
            if (flag & FIELD_SKILL_INSTRUMENT)
            {
                handle = field_find_free_inventory_record();
                if (handle != 0)
                {
                    field_copy_inventory_record(handle,
                                                (((u8*)g_saved_game_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) + 0x90)));
                    off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250);
                    flag_ptr = (u8*)g_saved_game_ctx + off;
                    flag_ptr[0x640] = 0;
                    field_compact_inventory(off);
                }
                else
                {
                    g_menu_message_line1 = (void*)menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_CANNOT_REMOVE);
                    i = MENU_SLOT_COUNT - 1;
                    while (i >= 0)
                    {
                        g_menu_slots[i].active = 0;
                        i--;
                    }
                    menu_open_content_page(MENU_PAGE_MESSAGE);
                    return buf;
                }
            }
            flag_ptr = (u8*)g_saved_game_ctx + (g_menu_char_slot * 0x250) + g_menu_active_subtype;
            *(flag_ptr + (MENU_SKILL_OFFSET - MENU_SKILL_SUBTYPE_BASE)) = FIELD_SKILL_NONE;
            list->active = MENU_SLOT_STATE_CLOSING;
            break;
        }

        case 3:
            if (g_menu_active_equipped_item->active == 0)
            {
                break;
            }
            i3 = MENU_SLOT_COUNT - 1;
            while (i3 >= 0)
            {
                g_menu_slots[i3].active = 0;
                i3--;
            }
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = 0x1A;
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = 0x17;
            g_menu_hit_item_idx = menu_find_active_content_item();
            if (g_menu_hit_item_idx != -1)
            {
                items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                g_content_view_x = items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_X_MASK;
                g_content_view_y = items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET;
                g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                g_menu_cursor_enable = 1;
            }
            g_menu_draw_early_out = 1;
            return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TECHNIQUE), 1, 0x30 - view_origin->x, -view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_MAGIC), 1, 0x30 - view_origin->x, 0x10 - view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_REMOVE), 1, 0x30 - view_origin->x, 0x20 - view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_STATUS), 1, 0x30 - view_origin->x, 0x30 - view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draws the confirmation page and dismisses it on any cancel/confirm press.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x88 - x, -y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    void* buf;

    buf = prim_buf;

    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        state->anim_frame = 0;
        state->active = 0;
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
    }

    buf = field_draw_text(buf, ot, g_menu_message_line1, 1, 0x88 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draws the two-line confirmation page and dismisses it on cancel/confirm.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x88 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_two_line_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    void* buf;

    buf = prim_buf;

    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        state->anim_frame = 0;
        state->active = 0;
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
    }

    buf = field_draw_text(buf, ot, g_menu_message_line1, 1, 0x88 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, g_menu_message_line2, 1, 0x88 - view_origin->x, 0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draws a two-glyph scroll-list page and handles its cancel/confirm input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_item_followup_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuContentItem* item;
    uintptr_t tbl;
    ScrollListState* list;
    Vec2s pos;
    s32 i;
    void* buf;
    s32 idx;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        if (list->navigation.fields.selected_index != 0)
        {
            list->active = MENU_SLOT_STATE_CLOSING;
            *(u8*)g_menu_item_ptr = 0;
            field_compact_inventory();
            g_menu_pending_item_row = 0xFF;
        }
        else
        {
            g_menu_scene_type += 1;
            g_menu_active_node = g_menu_scene_type;
            for (i = 3; i >= 0; i--)
            {
                g_menu_slots[i].active = 0;
            }
            idx = menu_find_active_content_item();
            g_menu_hit_item_idx = idx;
            if (idx != -1)
            {
                tbl = (uintptr_t)g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                item = (MenuContentItem*)((idx * sizeof(MenuContentItem)) + tbl);
                g_content_view_x = item->packed_x & MENU_CONTENT_X_MASK;
                g_content_view_y = item->y - MENU_CONTENT_VIEW_Y_OFFSET;
                g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                g_menu_cursor_enable = 1;
            }
        }
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_ITEM_STATUS), 1, 0x30 - view_origin->x, -view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TRASH), 1, 0x30 - view_origin->x, 0x10 - view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

