#include "internal/menu_internal.h"
#include "main/audio/akao.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_text.h"

s32 menu_find_active_content_item(void);
s32 menu_remove_technique_skills(void);

/** @brief Largest number menu_draw_clamped_number draws. */
#define MENU_NUMBER_MAX 99

/**
 * @brief Combine the type-conflict bits of the active character's equipment, skipping one slot.
 * @param excluded_slot Equipment slot to leave out.
 * @return Conflict bits of the other equipped items.
 */
extern inline s32 menu_get_equipment_conflicts(s32 excluded_slot)
{
    s32 equipment_index;
    s32 conflicts;
    MenuItemEntry* equipment;
    u32 item_info;

    conflicts = 0;
    equipment = g_menu_equipment_base;
    for (equipment_index = 0; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
    {
        if ((equipment_index != excluded_slot) && (equipment->active != 0))
        {
            item_info = equipment->attributes.packed;
            if (item_info & FIELD_ITEM_CATEGORY_BITS)
            {
                conflicts |= g_field_armor_type_conflicts[FIELD_ITEM_TYPE(item_info)];
            }
            else
            {
                conflicts |= g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(item_info)];
            }
        }

        equipment++;
    }
    return conflicts;
}

/**
 * @brief Locate the equipment record of the active character's active equipment slot.
 * @return Equipment record selected by g_menu_active_subtype.
 */
static inline MenuItemEntry* menu_active_equipment(void)
{
    return (MenuItemEntry*)((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                            ((g_menu_active_subtype << 6) + (MENU_CHARACTER_ITEMS_OFFSET - MENU_EQUIPMENT_SUBTYPE_BASE * MENU_ITEM_RECORD_SIZE)));
}

/**
 * @brief Locate the weapon record of the active character.
 * @return Equipment record in FIELD_WEAPON_SLOT.
 */
static inline MenuItemEntry* menu_weapon_record(void)
{
    return (MenuItemEntry*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (uintptr_t)g_saved_game_ctx + MENU_FIRST_EQUIPMENT_OFFSET);
}

/**
 * @brief Show a one-line message window in place of the open windows.
 * @param message MENU_TEXT_MESSAGES entry to show.
 */
static inline void menu_show_message(s32 message)
{
    g_menu_message_line1 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), message);
    menu_clear_slots();
    menu_open_content_page(MENU_PAGE_MESSAGE);
}

/**
 * @brief Draw the Equip / Auto-Select / Total-Auto / Status / Remove window of an equipment slot and run the chosen command.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_equipment_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    ScrollListState* list;
    MenuContentItem* item;
    uintptr_t content_table;
    u8* armor_record;
    MenuItemEntry* bare_hands;
    u8* saved_game;
    u8* equipment_base;
    s32 character_offset;
    u32 weapon_info;
    u32 bare_hands_info;
    s32 hit;
    void* buf;
    MenuItemEntry* free_record;
    MenuItemEntry* shield_record;
    s32 armor_offset;
    s32 equipment_offset;
    s32 equipment_index;

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
                switch (g_menu_active_subtype)
                {
                    case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                    {
                        menu_clear_slots();
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x24;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id =
                            D_8016869F[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx];

                        g_menu_equipment_conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
                        g_menu_content_ready = 1;
                        menu_open_content_page(MENU_PAGE_WEAPONS);
                        g_menu_draw_early_out = 1;
                        return buf;
                    }

                    case MENU_EQUIPMENT_SUBTYPE(1):
                    case MENU_EQUIPMENT_SUBTYPE(2):
                    case MENU_EQUIPMENT_SUBTYPE(3):
                    {
                        menu_clear_slots();
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x27;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id =
                            D_8016869B[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx];

                        g_menu_equipment_conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
                        g_menu_content_ready = 1;
                        menu_open_content_page(MENU_PAGE_ARMOR);
                        g_menu_draw_early_out = 1;
                        return buf;
                    }

                    default:
                        break;
                }
                break;

            case 1:
                if (g_menu_active_subtype == MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT))
                {
                    if (menu_stage_best_weapon() == 0)
                    {
                        break;
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
                    break;
                }

                g_item_slot_data[3] = 0;
                g_item_slot_data[2] = 0;
                g_item_slot_data[1] = 0;
                if (menu_stage_best_armor() == 0)
                {
                    break;
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
                break;

            case 2:
                g_item_slot_data[3] = 0;
                g_item_slot_data[2] = 0;
                g_item_slot_data[1] = 0;
                menu_stage_best_weapon();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(1);
                menu_stage_best_armor();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(2);
                menu_stage_best_armor();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(3);
                menu_stage_best_armor();

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
                break;

            case 3:
                if (g_menu_active_equipped_item->active == 0)
                {
                    break;
                }

                menu_clear_slots();
                switch (g_menu_active_subtype)
                {
                    case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                        g_menu_item_ptr = g_menu_active_equipped_item;
                        g_menu_category0_item = g_menu_saved_category0_item;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x14;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = 0x12;
                        break;

                    case MENU_EQUIPMENT_SUBTYPE(1):
                    case MENU_EQUIPMENT_SUBTYPE(2):
                    case MENU_EQUIPMENT_SUBTYPE(3):
                        g_menu_item_ptr = g_menu_active_equipped_item;
                        g_menu_category1_item = g_menu_saved_category1_item;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x17;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = 0x15;
                        break;
                }

                hit = menu_find_active_content_item();
                g_menu_hit_item_idx = hit;
                if (hit != -1)
                {
                    content_table = (uintptr_t)g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                    item = (MenuContentItem*)((hit * sizeof(MenuContentItem)) + content_table);
                    g_content_view_x = item->packed_x & MENU_CONTENT_X_MASK;
                    g_content_view_y = item->y - MENU_CONTENT_VIEW_Y_OFFSET;
                    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                    g_menu_cursor_enable = 1;
                }

                g_menu_draw_early_out = 1;
                return buf;

            case 4:
                if (g_menu_active_equipped_item->active == 0)
                {
                    break;
                }

                free_record = field_find_free_inventory_record();
                if (free_record != NULL)
                {
                    if (menu_item_differs_from_bare_hands(menu_active_equipment()) != 0)
                    {
                        field_copy_inventory_record(free_record, menu_active_equipment());
                        switch (g_menu_active_subtype)
                        {
                            case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                                g_menu_active_equipped_item = NULL;
                                g_menu_saved_category0_item = NULL;
                                character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                                saved_game = (u8*)g_saved_game_ctx;
                                weapon_info =
                                    ((MenuCharacterRecord*)(saved_game + character_offset + MENU_CHARACTER_RECORD_OFFSET))->items[0].attributes.packed >>
                                    FIELD_ITEM_TYPE_SHIFT;
                                bare_hands = &g_field_bare_hands_item;
                                bare_hands_info = bare_hands->attributes.packed >> FIELD_ITEM_TYPE_SHIFT;

                                /* Bare hands are a glove: removing a glove changes nothing else. */
                                if ((weapon_info & FIELD_ITEM_TYPE_MASK) == (bare_hands_info & FIELD_ITEM_TYPE_MASK))
                                {
                                    field_copy_inventory_record(
                                        &menu_character_record((SavedGameLayout*)saved_game, g_menu_char_slot)->items[FIELD_WEAPON_SLOT], bare_hands);
                                    break;
                                }

                                field_copy_inventory_record(&menu_character_record((SavedGameLayout*)saved_game, g_menu_char_slot)->items[FIELD_WEAPON_SLOT],
                                                            bare_hands);

                                /* Bare hands cannot hold a shield: move it to the inventory, or undo the removal when there is no room. */
                                for (equipment_index = FIELD_ARMOR_SLOT; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
                                {
                                    armor_offset = equipment_index << 6;
                                    armor_record = (u8*)g_saved_game_ctx + (armor_offset + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE));
                                    if (armor_record[MENU_FIRST_EQUIPMENT_OFFSET] != 0)
                                    {
                                        if (FIELD_ITEM_TYPE(*(u32*)(armor_record + MENU_FIRST_EQUIPMENT_OFFSET + OFFSETOF(MenuItemEntry, attributes))) ==
                                            FIELD_ARMOR_SHIELD)
                                        {
                                            shield_record = field_find_free_inventory_record();
                                            if (shield_record != NULL)
                                            {
                                                field_copy_inventory_record(
                                                    shield_record,
                                                    (((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET)) +
                                                     (armor_offset + MENU_CHARACTER_ITEMS_OFFSET)));
                                                menu_equipped_item(g_saved_game_ctx, equipment_index)->active = 0;
                                                field_compact_inventory();
                                                break;
                                            }

                                            field_copy_inventory_record(menu_weapon_record(), free_record);
                                            free_record->active = 0;
                                            menu_show_message(MENU_MESSAGE_CANNOT_REMOVE);
                                            return buf;
                                        }
                                    }
                                }

                                if (menu_remove_technique_skills() == 0)
                                {
                                    break;
                                }

                                menu_show_message(MENU_MESSAGE_TECHNIQUES_REMOVED);
                                return buf;

                            case MENU_EQUIPMENT_SUBTYPE(1):
                            case MENU_EQUIPMENT_SUBTYPE(2):
                            case MENU_EQUIPMENT_SUBTYPE(3):
                                g_menu_active_equipped_item = NULL;
                                g_menu_saved_category1_item = NULL;
                                {
                                    s32 subtype;

                                    subtype = g_menu_active_subtype;
                                    equipment_base = (u8*)g_saved_game_ctx;
                                    equipment_offset = ((subtype - MENU_EQUIPMENT_SUBTYPE_BASE) << 6) + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                                    *(equipment_base + equipment_offset + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                                    field_compact_inventory();
                                }
                                break;
                        }
                    }

                    list->active = MENU_SLOT_STATE_CLOSING;
                    break;
                }

                menu_show_message(MENU_MESSAGE_CANNOT_REMOVE);
                return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_EQUIP), 1, 0x30 - view_origin->x, -view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_AUTO_SELECT), 1, 0x30 - view_origin->x,
                          0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TOTAL_AUTO), 1, 0x30 - view_origin->x,
                          0x20 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_STATUS), 1, 0x30 - view_origin->x,
                          0x30 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_REMOVE), 1, 0x30 - view_origin->x,
                          0x40 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draw the Change / Cancel window that follows Auto-Select and Total-Auto, and keep or undo the staged equipment.
 * @param ot Ordering-table pointer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input.
 * @return Updated primitive write cursor; the unchanged @p prim_buf when the window closes.
 */
void* menu_equipment_compare_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    ScrollListState* list;
    s32 equipment_index;
    MenuItemEntry* replaced_item;
    s32 character_offset;
    void* buf;

    list = state;
    buf = prim_buf;
    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        g_menu_compare_window_active = 0;

        if ((list->navigation.fields.selected_index != 0) || (g_pad_input & PAD_BTN_CIRCLE))
        {
            /* Cancel: put every staged slot back the way it was. */
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            for (equipment_index = 0; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
            {
                if (g_item_slot_flags[equipment_index] != 0)
                {
                    replaced_item = g_item_slot_data[equipment_index];
                    if (replaced_item != NULL)
                    {
                        menu_swap_item_records(replaced_item, (MenuItemEntry*)(((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) +
                                                                                                         MENU_CHARACTER_RECORD_OFFSET)) +
                                                                               ((equipment_index << 6) + MENU_CHARACTER_ITEMS_OFFSET)));
                    }
                    else if (equipment_index == FIELD_WEAPON_SLOT)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), menu_weapon_record());
                        field_copy_inventory_record(menu_weapon_record(), &g_field_bare_hands_item);
                    }
                    else
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(),
                                                    (((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET)) +
                                                     ((equipment_index << 6) + MENU_CHARACTER_ITEMS_OFFSET)));
                        character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                        *((u8*)g_saved_game_ctx + ((equipment_index << 6) + character_offset) + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                    }
                }
            }
        }
        else
        {
            /* Change: keep the staged equipment; a new weapon type invalidates the assigned techniques. */
            menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            if (g_item_slot_flags[FIELD_WEAPON_SLOT] != 0)
            {
                if (((g_item_slot_data[FIELD_WEAPON_SLOT] != NULL) && ((g_item_slot_data[FIELD_WEAPON_SLOT]->attributes.packed & FIELD_ITEM_TYPE_BITS) !=
                                                                       (g_menu_equipment_base->attributes.packed & FIELD_ITEM_TYPE_BITS))) ||
                    ((g_item_slot_data[FIELD_WEAPON_SLOT] == NULL) &&
                     ((g_field_bare_hands_info & FIELD_ITEM_TYPE_BITS) != (g_menu_equipment_base->attributes.packed & FIELD_ITEM_TYPE_BITS))))
                {
                    if (menu_remove_technique_skills() != 0)
                    {
                        menu_show_message(MENU_MESSAGE_TECHNIQUES_REMOVED);
                    }
                }
            }
        }

        field_compact_inventory();
        g_item_slot_flags[0] = 0;
        g_item_slot_flags[1] = 0;
        g_item_slot_flags[2] = 0;
        g_item_slot_flags[3] = 0;
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_CHANGE), 1, 0x30 - view_origin->x, -view_origin->y,
                          FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_CANCEL), 1, 0x30 - view_origin->x,
                          0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    g_pad_input = 0;
    return buf;
}

/**
 * @brief Test whether an item record differs from FIELD's BareHands record.
 * @param item_record Item record to compare.
 * @return 1 as soon as a byte differs; 0 if the whole record is equal.
 */
inline s32 menu_item_differs_from_bare_hands(const MenuItemEntry* item_record)
{
    const u8* item;
    const u8* bare_hands;
    u32 i;

    item = (const u8*)item_record;
    bare_hands = (const u8*)&g_field_bare_hands_item;

    for (i = 0; i < MENU_ITEM_RECORD_SIZE; i++, bare_hands++, item++)
    {
        if (*bare_hands != *item)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Swap two item records through a stack buffer.
 * @param first_item First item record.
 * @param second_item Second item record.
 */
inline void menu_swap_item_records(MenuItemEntry* first_item, MenuItemEntry* second_item)
{
    u8 swap_buffer[MENU_ITEM_RECORD_SIZE];

    field_copy_inventory_record(swap_buffer, first_item);
    field_copy_inventory_record(first_item, second_item);
    field_copy_inventory_record(second_item, swap_buffer);
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
 * @brief Draw the learned Special Technique list and assign the chosen technique to the active skill slot.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state_arg Scroll-list state for this page.
 * @param prim_arg Primitive buffer write cursor.
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @note Only techniques of the equipped weapon's type can be assigned.
 */
void* menu_special_technique_list_callback(u_long* ot, ScrollListState* state_arg, void* prim_arg, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = state_arg;
    void* prim = prim_arg;
    s32 selected_technique;
    s32 list_y;
    s32 weapon_type;
    s32 technique_index;
    s32 bit_mask;
    s32 learned_techniques;
    s32* learned_ptr;
    s32 relative_y;
    s32 scroll_y;
    u8 skill;
    MenuItemEntry* free_record;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        state->active = MENU_SLOT_STATE_CLOSING;
        return prim;
    }

    selected_technique = -1;
    list_y = 0;
    weapon_type = 0;
    learned_ptr = (s32*)g_saved_game_ctx->technique_bits;
    while (weapon_type < FIELD_WEAPON_CATEGORY_COUNT)
    {
        technique_index = 0;
        bit_mask = 1;
        learned_techniques = *learned_ptr;
        while (technique_index < FIELD_TECHNIQUES_PER_WEAPON)
        {
            if (learned_techniques & bit_mask)
            {
                if (state->navigation.fields.selected_index == (list_y >> 4))
                {
                    selected_technique = technique_index + (weapon_type * FIELD_TECHNIQUES_PER_WEAPON);
                }
                list_y += MENU_ITEM_NAV_POSITION_STRIDE;
            }
            technique_index++;
            bit_mask <<= 1;
        }
        weapon_type++;
        learned_ptr++;
    }

    if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        if ((selected_technique / FIELD_TECHNIQUES_PER_WEAPON) == (s32)FIELD_ITEM_TYPE(g_menu_equipment_base->attributes.packed))
        {
            skill = g_saved_game_ctx->characters[g_menu_char_slot].info.actions.skills[g_menu_active_subtype - MENU_SKILL_SUBTYPE_BASE];
            if (skill != FIELD_SKILL_NONE)
            {
                /* The slot holds an instrument: return its item record to the inventory first. */
                if (skill & FIELD_SKILL_INSTRUMENT)
                {
                    free_record = field_find_free_inventory_record();
                    if (free_record != NULL)
                    {
                        s32 item_offset;

                        field_copy_inventory_record(free_record, (u8*)g_saved_game_ctx +
                                                                     ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                                                                     ((g_menu_active_subtype << 6) + (MENU_CHARACTER_ITEMS_OFFSET + MENU_ITEM_RECORD_SIZE)));
                        item_offset = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                        *((u8*)g_saved_game_ctx + item_offset + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                        field_compact_inventory();
                    }
                    else
                    {
                        menu_show_message(MENU_MESSAGE_CANNOT_REMOVE);
                        return prim;
                    }
                }
            }

            g_saved_game_ctx->characters[g_menu_char_slot].info.actions.skills[g_menu_active_subtype - MENU_SKILL_SUBTYPE_BASE] =
                selected_technique % FIELD_TECHNIQUES_PER_WEAPON;
            menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
            state->active = MENU_SLOT_STATE_CLOSING;
        }
        else
        {
            menu_play_se(SFX_ERROR, AKAO_SFX_DEFAULT_VOLUME);
        }
    }

    prim = scroll_list_draw(prim, ot, state, g_menu_scroll_nav_entries, view_origin, active);

    selected_technique = -1;
    list_y = 0;
    weapon_type = 0;
    learned_ptr = (s32*)g_saved_game_ctx->technique_bits;
    scroll_y = state->scroll_y;
    while (weapon_type < FIELD_WEAPON_CATEGORY_COUNT)
    {
        technique_index = 0;
        bit_mask = 1;
        while (technique_index < FIELD_TECHNIQUES_PER_WEAPON)
        {
            if (*learned_ptr & bit_mask)
            {
                relative_y = list_y - scroll_y;
                if (relative_y >= -0xF)
                {
                    if (relative_y < state->viewport_h - 0x10)
                    {
                        s32 glyph_color;
                        u8* name_table;
                        s32 name_offset;
                        u8* name;

                        /* Techniques of the equipped weapon's type are drawn in the highlight color. */
                        glyph_color = 3;
                        name_table = g_menu_state_ptr;
                        name_table += ((MenuTextResources*)name_table)->table_offsets[MENU_TEXT_TECHNIQUE_NAMES];
                        name_offset = *(u16*)((u8*)name_table + (technique_index * 2) + (weapon_type * (FIELD_TECHNIQUES_PER_WEAPON * 2)));
                        name = name_table + name_offset;
                        if (weapon_type == (s32)FIELD_ITEM_TYPE(g_menu_equipment_base->attributes.packed))
                        {
                            glyph_color = 1;
                        }
                        prim = field_draw_text(prim, ot, name, glyph_color, 0x10 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                    }
                }

                if (state->navigation.fields.selected_index == (list_y >> 4))
                {
                    selected_technique = technique_index + (weapon_type * FIELD_TECHNIQUES_PER_WEAPON);
                }
                list_y += MENU_ITEM_NAV_POSITION_STRIDE;
            }
            technique_index++;
            bit_mask <<= 1;
        }
        weapon_type++;
        learned_ptr++;
    }

    if (selected_technique != -1)
    {
        u8* help_table = g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_TECHNIQUE_HELP];

        g_menu_help_text = (u8*)(help_table + ((u16*)help_table)[selected_technique]);
    }
    return prim;
}

/**
 * @brief Open the menu content window for the given content page.
 * @param content_id MenuContentPage to open; any other value only resets g_menu_pending_item_row.
 */
void menu_open_content_page(u32 content_id)
{
    MenuSlotRect rect;
    MenuSlot* slot;
    s32 entry_count;

    g_menu_pending_item_row = MENU_NONE;
    switch (content_id)
    {
        case MENU_PAGE_WEAPONS:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_WEAPON);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_WEAPON;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ARMOR:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_ARMOR);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_ARMOR;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_INSTRUMENTS:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_INSTRUMENT);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_INSTRUMENT;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_EVENTS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x90;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_event_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_event_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ITEMS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x80;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_item_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_item_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ARTIFACTS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x90;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_artifact_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_artifact_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_MESSAGE:
            rect.x = 0x10;
            rect.y = 0x60;
            rect.w = 0x120;
            rect.h = 0x20;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_message_callback;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, 1);

            menu_relink_item_nav(1);
            g_menu_draw_early_out = 1;
            break;

        case MENU_PAGE_TWO_LINE_MESSAGE:
            rect.x = 0x10;
            rect.y = 0x60;
            rect.w = 0x120;
            rect.h = 0x30;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_two_line_message_callback;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, 1);

            menu_relink_item_nav(1);
            g_menu_draw_early_out = 1;
            break;
    }
}

/**
 * @brief Build circular navigation for the Event List: every event that has reached its listed stage.
 * @return Number of listed events.
 */
s32 menu_build_event_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 event_count;
    s32 word_index;
    s32 event_index;
    u32 stages;
    s32 stage_mask;

    event_count = 0;
    word_index = 0;
    stage_mask = MENU_EVENT_STAGE_MASK;
    while (word_index < MENU_EVENT_WORD_COUNT)
    {
        stages = g_saved_game_ctx->words[SAVED_GAME_WORD_EVENT_PROGRESS + word_index];
        event_index = MENU_EVENTS_PER_WORD - 1;
        while (event_index >= 0)
        {
            if ((stages & stage_mask) >= MENU_EVENT_STAGE_LISTED)
            {
                event_count++;
            }
            event_index--;
            stages = stages >> MENU_EVENT_STAGE_BITS;
        }
        word_index++;
    }

    g_menu_scroll_nav_entries[0] = 0;
    for (event_index = 0; event_index < event_count; event_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + event_index;

        packed_entry = *entry;
        previous_index = event_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((event_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = event_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = event_index + 1;
        wrapped_next_index = 0;

        if (next_index < event_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return event_count;
}

/**
 * @brief Clear the unused last item kind and build circular navigation for the Items list.
 * @return Number of item kinds held.
 */
s32 menu_build_item_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 item_count;
    s32 remaining_kinds;
    s32 entry_index;
    u8* quantity;

    item_count = 0;
    g_saved_game_ctx->item_counts[FIELD_ITEM_KIND_COUNT - 1] = 0;
    quantity = g_saved_game_ctx->item_counts;
    for (remaining_kinds = FIELD_ITEM_KIND_COUNT - 1; remaining_kinds >= 0; remaining_kinds--)
    {
        if (*quantity != 0)
        {
            item_count++;
        }
        quantity++;
    }

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
 * @brief Build circular navigation for the Artifacts list: one entry per placed land.
 * @return Number of placed lands.
 */
s32 menu_build_artifact_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 land_count;
    s32 remaining_lands;
    s32 entry_index;
    FieldLandRecord* land;

    land_count = 0;
    land = g_saved_game_ctx->lands;
    for (remaining_lands = FIELD_LAND_COUNT - 1; remaining_lands >= 0; remaining_lands--)
    {
        if (land->flags & FIELD_LAND_PLACED)
        {
            land_count++;
        }
        land++;
    }

    g_menu_scroll_nav_entries[0] = 0;
    for (entry_index = 0; entry_index < land_count; entry_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + entry_index;

        packed_entry = *entry;
        previous_index = entry_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = land_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;

        if (next_index < land_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return land_count;
}

/**
 * @brief Stage the best weapon from the inventory in the weapon slot for comparison.
 * @return 1 when a weapon was staged, otherwise 0.
 */
s32 menu_stage_best_weapon(void)
{
    MenuItemEntry* candidate;

    menu_stage_stack_shape(0, 0, 0, 0, 0, 0);

    candidate = menu_find_best_weapon();
    if (candidate != NULL)
    {
        if (menu_item_differs_from_bare_hands(menu_weapon_record()) == 0)
        {
            field_copy_inventory_record(&menu_character_record(g_saved_game_ctx, g_menu_char_slot)->items[FIELD_WEAPON_SLOT], candidate);
            candidate->active = 0;
            g_item_slot_data[FIELD_WEAPON_SLOT] = NULL;
        }
        else
        {
            menu_swap_item_records(menu_weapon_record(), candidate);
            g_item_slot_data[FIELD_WEAPON_SLOT] = candidate;
        }
        g_item_slot_flags[FIELD_WEAPON_SLOT] = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Find the inventory weapon with the highest power that does not conflict with the equipped armor.
 * @return First weapon stronger than the equipped one, or NULL if none is.
 */
MenuItemEntry* menu_find_best_weapon(void)
{
    s32 item_index;
    s32 conflicts;
    s32 item_value;
    s32 best_value;
    u32 item_info;
    MenuItemEntry* item;
    MenuItemEntry* best_item;

    best_value = 0;
    best_item = &menu_character_record(g_saved_game_ctx, g_menu_char_slot)->items[FIELD_WEAPON_SLOT];
    if (best_item->active != 0)
    {
        best_value = best_item->stat_values[0];
    }
    item = (MenuItemEntry*)g_saved_game_ctx->items;
    best_item = NULL;
    conflicts = menu_get_equipment_conflicts(FIELD_WEAPON_SLOT);
    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->active != 0)
        {
            item_info = item->attributes.packed;
            if (!(item_info & FIELD_ITEM_CATEGORY_BITS) && !(conflicts & g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(item_info)]))
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
 * @brief Stage the best armor from the inventory in the active armor slot for comparison.
 * @return 1 if an armor was staged, otherwise 0.
 */
s32 menu_stage_best_armor(void)
{
    MenuItemEntry* candidate;
    MenuItemEntry** staged_items;
    SavedGameLayout* saved_game;
    s32 armor_slot;
    s32 slot_index;

    if (0)
    {
        field_copy_inventory_record(0, 0, 0, 0, 0, 0);
    }

    candidate = menu_find_best_armor();
    if (candidate != NULL)
    {
        slot_index = FIELD_ARMOR_SLOT;
        armor_slot = g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE;
        saved_game = g_saved_game_ctx;
        if (menu_equipped_item(saved_game, armor_slot)->active == 0)
        {
            /* A candidate staged for another slot is now the record being replaced here. */
            staged_items = &g_item_slot_data[0];
            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index++)
            {
                if (candidate == staged_items[slot_index])
                {
                    staged_items[slot_index] = menu_active_equipment();
                    break;
                }
            }
            field_copy_inventory_record(menu_active_equipment(), candidate);
            candidate->active = 0;
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = NULL;
        }
        else
        {
            staged_items = &g_item_slot_data[0];
            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index++)
            {
                if (candidate == staged_items[slot_index])
                {
                    staged_items[slot_index] = menu_active_equipment();
                    break;
                }
            }
            menu_swap_item_records((MenuItemEntry*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (uintptr_t)g_saved_game_ctx +
                                                    (g_menu_active_subtype << 6) +
                                                    (MENU_FIRST_EQUIPMENT_OFFSET - MENU_EQUIPMENT_SUBTYPE_BASE * MENU_ITEM_RECORD_SIZE)),
                                   candidate);
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = candidate;
        }
        g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Find the inventory armor for the active armor slot with the highest sum of its four defense values.
 * @return First armor better than the equipped one that does not conflict with the other equipment, or NULL if none is.
 */
MenuItemEntry* menu_find_best_armor(void)
{
    s32 item_index;
    s32 conflicts;
    s32 item_value;
    s32 best_value;
    u32 item_info;
    MenuItemEntry* item;
    MenuItemEntry* best_item;

    best_item = &menu_character_record(g_saved_game_ctx, g_menu_char_slot)->items[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE];
    if (best_item->active == 0)
    {
        best_value = 0;
    }
    else
    {
        best_value = best_item->stat_values[0] + best_item->stat_values[1] + best_item->stat_values[2] + best_item->stat_values[3];
    }
    best_item = NULL;
    item = (MenuItemEntry*)g_saved_game_ctx->items;
    conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->active != 0)
        {
            item_info = item->attributes.packed;
            if (((item_info & FIELD_ITEM_CATEGORY_BITS) == (FIELD_ITEM_CATEGORY_ARMOR << FIELD_ITEM_CATEGORY_SHIFT)) &&
                !(conflicts & g_field_armor_type_conflicts[FIELD_ITEM_TYPE(item_info)]))
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

/**
 * @brief Play a menu sound effect, unless a menu script is currently driving input.
 * @param sound_id Sound effect ID, such as SFX_CURSOR.
 * @param volume Playback volume (menu callers always pass AKAO_SFX_DEFAULT_VOLUME).
 */
void menu_play_se(s32 sound_id, s32 volume)
{
    if (g_active_script == 0)
    {
        field_play_sound(sound_id, volume);
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

    item = (MenuItemEntry*)g_saved_game_ctx->items;
    for (count = 0; count < MENU_ITEM_TABLE_COUNT; count++)
    {
        if (item->active == 0)
        {
            break;
        }
        item++;
    }
    return count;
}

/**
 * @brief Draw a number, showing values above MENU_NUMBER_MAX as MENU_NUMBER_MAX.
 * @param ot_entry Ordering-table entry for the emitted primitives.
 * @param packet_cursor Primitive write cursor.
 * @param value Number to draw; negative values pass through.
 * @param format Renderer option, passed through; meaning unconfirmed.
 * @param origin Number position, passed through to the renderer.
 * @param style Renderer style, passed through unchanged.
 * @return Updated primitive write cursor.
 */
void* menu_draw_clamped_number(u_long* ot_entry, void* packet_cursor, s32 value, s32 format, Vec2s* origin, s32 style)
{
    if (value > MENU_NUMBER_MAX)
    {
        value = MENU_NUMBER_MAX;
    }
    return field_draw_number(ot_entry, packet_cursor, value, format, origin, style);
}
