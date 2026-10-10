#include "overlays/field/field_text.h"
#include "internal/niki_internal.h"
#include "overlays/menu/menu.h"


/**
 * @brief Initialize card browsing, drawing resources, and the selected menu mode.
 * @param context_value Caller value retained for the overlay; its meaning is unresolved.
 * @param mode Menu mode, with zero selecting the entry browser.
 */
void niki_init(s32 context_value, s32 mode)
{
    RECT rect;

    g_card_menu_mode = mode;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;
    card_reset_entry_ranks();
    card_menu_init_card_events();
    g_card_menu_icon_phase = 0;
    field_set_default_fade_target();
    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();
    g_card_menu_write_in_progress = 0;
    g_card_menu_progress_active = 0;
    g_card_menu_selection_status = 0;
    g_card_menu_io_busy = 0;
    g_card_menu_frame_parity = 0;
    g_card_menu_exit_requested = 0;
    field_reset_input_repeat();
    niki_build_ui_elements();
    D_80164AE4 = context_value;
}

#include "../../common/card_menu/card_menu_update_frame.inc.c"

/** @brief Reset selection and create the windows for the active menu mode. */
void niki_build_ui_elements(void)
{
    CardMenuElement* element;
    s32 unused[2];

    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_card_menu_selection_status = 0;
    g_niki_items = g_saved_game_ctx->items;
    card_menu_clear_elements();
    D_80164B80 = 0;
    if (g_card_menu_mode != 0)
    {
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        element = card_menu_alloc_element();
        element->draw = niki_draw_state_page;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);

        element = card_menu_alloc_element();
        element->draw = card_menu_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = card_menu_alloc_element();
        element->draw = card_menu_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        return;
    }

    g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element = card_menu_alloc_element();
    element->draw = card_menu_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_LIST_X;
    element->attr.bits.y = CARD_MENU_LIST_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_LIST_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_LIST_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_LIST_WIDTH);

    element = card_menu_alloc_element();
    element->draw = card_menu_draw_mode_title;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_TITLE_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_TITLE_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_TITLE_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_TITLE_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_TITLE_WIDTH);

    element = card_menu_alloc_element();
    element->draw = card_menu_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = card_menu_alloc_element();
    element->draw = card_menu_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = card_menu_alloc_element();
    element->draw = niki_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_DETAILS_X;
    element->attr.bits.y = CARD_MENU_DETAILS_Y;
    element->size.bits.width_high = CARD_MENU_DETAILS_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_DETAILS_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DETAILS_WIDTH);
    g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

#include "../../common/card_menu/card_menu_update_state.inc.c"

/**
 * @brief Run immediate sequence steps, then select the next wait or recovery sequence.
 * @return Unspecified; callers ignore the return value.
 */
s32 card_menu_update_card_sequence(void)
{
    s32 result;

    if (g_card_entry_state >= CARD_MENU_ENTRY_COUNT_LIMIT)
    {
        if (g_card_step == NULL)
        {
            g_card_step = g_card_steps_initial_scan;
        }
    }

    do
    {
        result = niki_advance_load_sequence();
    } while (result == CARD_MENU_SEQUENCE_RUN_AGAIN);

    if ((D_80164B80 != 0) && (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK))
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
        g_card_step = g_card_steps_card_info;
    }
    else
    {
        switch (result)
        {
        case CARD_MENU_SEQUENCE_NONE:
            break;
        case CARD_MENU_SEQUENCE_NO_CARD:
            g_card_step = g_card_steps_rescan;
            D_80164B80 = 0;
            break;
        case CARD_MENU_SEQUENCE_UNFORMATTED:
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
            /* fallthrough */
        case CARD_MENU_SEQUENCE_FINISHED:
            g_card_step = g_card_steps_card_info;
            break;
        }
    }
}

/**
 * @brief Handle card switching, entry navigation, cancellation, and load confirmation.
 * @return Unspecified; callers ignore the return value.
 * @return Unspecified; callers ignore the return value.
 */
s32 card_menu_handle_input(void)
{
    s32 entry_count;
    s32 status;
    s32 navigation_steps;
    CardMenuElement* element;

    if ((g_card_menu_element_pool[1].attr.word & CARD_MENU_ELEMENT_STATE_MASK) == 0)
    {
        g_card_menu_exit_requested = 1;
        return;
    }
    if (g_card_menu_exit_requested != 0)
    {
        return;
    }
    if (((s32)g_card_menu_element_pool[1].attr.word & CARD_MENU_ELEMENT_STATE_MASK) >= 3)
    {
        return;
    }
    if ((g_card_menu_element_pool[0].attr.word & CARD_MENU_ELEMENT_STATE_MASK) != 0)
    {
        return;
    }
    entry_count = g_card_entry_state;
    if (entry_count == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
    {
        return;
    }
    if (g_card_menu_entry_scan_active != 0)
    {
        return;
    }
    if (g_card_menu_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_card_step - 6) < 2U)
    {
        return;
    }
    if (g_card_menu_mode != 0)
    {
        return;
    }

    status = g_pad_input;
    if (status & NIKI_CANCEL_INPUT_MASK)
    {
        g_field_niki_addhero_state = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        card_menu_close_all_elements();
        return;
    }
    if (status & CARD_MENU_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        niki_switch_card_slot();
        return;
    }
    if (entry_count >= 0x10)
    {
        return;
    }

    navigation_steps = 1;
    if (status & 8)
    {
        g_pad_input = 0x4000;
        navigation_steps = 1;
    }
    if (g_pad_input & 4)
    {
        g_pad_input = 0x1000;
        navigation_steps = 1;
    }

    while (navigation_steps != 0)
    {
        if (g_pad_input & 0x1000)
        {
            g_card_menu_selected_row -= 1;
            if (g_card_menu_selected_row < 0)
            {
                g_card_menu_selected_row = g_card_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_card_menu_selected_row += 1;
            if (g_card_menu_selected_row >= g_card_entry_state)
            {
                g_card_menu_selected_row = 0;
            }
        }
        navigation_steps -= 1;
    }

    if (g_pad_input & 0x5000)
    {
        card_menu_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        card_menu_scroll_to_selection();
        return;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        if (g_card_menu_mode != 0)
        {
            return;
        }
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, 0xC) == 0)
        {
            SavedGameLayout* metadata = &g_niki_entry_file.saved_game;
            if ((metadata->identity.ids.game_id != g_saved_game_ctx->identity.ids.game_id) && (metadata->summary_slot_count != 0) &&
                ((g_save_compatibility_tag == SAVE_TAG_ANY) || (metadata->compatibility_tag == g_save_compatibility_tag)))
            {
                element = card_menu_alloc_element();
                element->attr.bits.transition_step = 1;
                element->attr.bits.x = CARD_MENU_MESSAGE_X;
                element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
                element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                element->size.bits.height = CARD_MENU_EXCHANGE_PROMPT_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);
                card_menu_enable_choice_toggle();
                element->draw = card_menu_draw_load_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    }
}

void niki_switch_card_slot(void)
{
    D_80164B80 = 0;
    g_card_step = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_card_menu_selection_status = 0;
    g_card_slot ^= 1;
    card_reset_entry_ranks();
}

#include "../../common/card_menu/card_menu_close_all_elements.inc.c"

#include "../../common/card_menu/card_menu_scroll_to_selection.inc.c"

#include "../../common/card_menu/card_menu_update_elements.inc.c"

#include "../../common/card_menu/card_menu_draw_entry_list.inc.c"

#include "../../common/card_menu/card_menu_draw_mode_title.inc.c"

#include "../../common/card_menu/card_menu_draw_card_slot0_label.inc.c"

#include "../../common/card_menu/card_menu_draw_card_slot1_label.inc.c"

/**
 * @brief Draw the niki save-slot detail panel: element glyphs, the playtime
 *        clock, the slot marker row, and a fallback name/second-line block.
 *
 * Runs only while the panel is active (g_card_menu_selection_status non-zero) and not suppressed
 * (g_card_menu_entry_scan_active zero). Depending on g_card_menu_selection_status it either emits a two-line caption
 * (state 2), or renders the full slot detail: up to three party markers laid out
 * by card_menu_draw_icon_highlight with an animated highlight (g_card_menu_icon_phase), the playtime split
 * into hours/minutes via field_draw_number, and one of three status glyphs. If the
 * slot compare fails it falls back to drawing the stored name and second line.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    Vec2s pos;
    u8 name[0x100];
    s32 icons[3];

    result = prim;
    if (g_card_menu_selection_status == 0)
    {
        return result;
    }
    if (g_card_menu_entry_scan_active != 0)
    {
        return result;
    }
    if (g_card_menu_selection_status != 3 && g_card_entry_state < 0x10)
    {
        if (g_card_menu_selection_status == 2)
        {
            s32 x = -x_offset;
            u16* text_table;

            result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_LEFT);
            text_table = CARD_MENU_TEXT_TABLE(g_niki_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE);
            return field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_USES_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, 0x10 - y_offset, FIELD_TEXT_ALIGN_LEFT);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, 0xC) == 0)
            {
                if (g_save_compatibility_tag == SAVE_TAG_ANY || g_niki_entry_file.saved_game.compatibility_tag == g_save_compatibility_tag)
                {
                    s32 icon_count;
                    s32 visible_icon_index;
                    s32 slot_index;
                    s32 base_icon_width;
                    s32 phase_span;
                    s32 phase_end;
                    s32 phase_start;
                    s32 icon_x;
                    s32 hours;
                    s32 wrapped_phase;

                    {
                        SavedGameLayout* record = &g_niki_entry_file.saved_game;
                        icons[0] = (u32)(record->spawn.word) >> 0x19;
                        icons[1] = (record->track.word >> 0x12) & 0x7F;
                        icons[2] = record->track.word >> 0x19;
                        g_card_menu_icon_palette = (s32)record->icon_palette;
                    }

                    icon_x = 0;
                    icon_count = 0;
                    for (visible_icon_index = 0; visible_icon_index < 3; visible_icon_index++)
                    {
                        if (icons[visible_icon_index] != 0x7F)
                        {
                            icon_count += 1;
                        }
                    }

                    switch (icon_count)
                    {
                    case 2:
                        base_icon_width = 0x20;
                        phase_span = 0x10;
                        wrapped_phase = g_card_menu_icon_phase;
                        if (g_card_menu_icon_phase < 0)
                        {
                            wrapped_phase = g_card_menu_icon_phase + 0x1F;
                        }
                        g_card_menu_icon_phase -= (wrapped_phase >> 5) << 5;
                        break;
                    case 3:
                        base_icon_width = 0x10;
                        phase_span = 0x20;
                        g_card_menu_icon_phase %= 0x60;
                        break;
                    default:
                        base_icon_width = 0x10;
                        phase_span = 0x20;
                        g_card_menu_icon_phase = 0x1F;
                        break;
                    }

                    visible_icon_index = 0;
                    slot_index = visible_icon_index;
                    for (; slot_index < 3; slot_index++)
                    {
                        phase_start = visible_icon_index * phase_span;
                        phase_end = phase_start + phase_span;
                        if (icons[slot_index] != 0x7F)
                        {
                            s32 icon_width = base_icon_width;
                            s32 wrapped_start;
                            s32 wrapped_end;
                            s32 delta;

                            if (g_card_menu_icon_phase >= phase_start && g_card_menu_icon_phase < phase_end)
                            {
                                delta = g_card_menu_icon_phase - phase_start;
                                icon_width += delta;
                            }
                            else
                            {
                                wrapped_start = phase_end % (phase_span * icon_count);
                                if (g_card_menu_icon_phase >= wrapped_start && g_card_menu_icon_phase < (wrapped_end = wrapped_start + phase_span))
                                {
                                    delta = wrapped_end - g_card_menu_icon_phase;
                                    icon_width += delta;
                                }
                            }
                            result = card_menu_draw_icon_highlight(result, ot, icon_x - x_offset, -y_offset, icon_width, icons[slot_index], visible_icon_index,
                                                              slot_index);
                            visible_icon_index += 1;
                            icon_x += icon_width;
                        }
                    }

                    {
                        SavedGameLayout* preview = &g_niki_entry_file.saved_game;
                        s32 x = -x_offset;
                        s32 y = -y_offset;
                        s32 playtime;

                        playtime = preview->play_time;
                        pos.x = (s16)(x + CARD_MENU_DETAILS_HOURS_RIGHT_X);
                        pos.y = (s16)y;
                        hours = playtime / 216000;
                        result = field_draw_number(ot, result, hours, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        result = field_draw_text(result, ot, (u8*)(g_text_time_separator_offset_bytes[0] + ((uintptr_t)&g_text_time_separator_offset_bytes - 0x32) + (g_text_time_separator_offset_bytes[1] << 8)), FIELD_TEXT_COLOR_NORMAL,
                                               x + CARD_MENU_DETAILS_TIME_SEPARATOR_X, y, FIELD_TEXT_ALIGN_LEFT);
                        playtime = (playtime / 3600) - (hours * 0x3C);
                        if (playtime < 0xA)
                        {
                            pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X);
                            pos.y = (s16)y;
                            result = field_draw_number(ot, result, 0, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        }
                        pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_RIGHT_X);
                        pos.y = (s16)y;
                        result = field_draw_number(ot, result, playtime, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        result = field_draw_text(result, ot, preview->summary_name, FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TEXT_X, y + 0x10, FIELD_TEXT_ALIGN_LEFT);

                        if (preview->identity.ids.game_id == g_saved_game_ctx->identity.ids.game_id)
                        {
                            result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_niki_text_same_hero_data, CARD_MENU_TEXT_SAME_HERO_DATA), FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, FIELD_TEXT_ALIGN_LEFT);
                        }
                        else if (preview->summary_slot_count == 0)
                        {
                            result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_niki_text_no_items, CARD_MENU_TEXT_NO_ITEMS), FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, FIELD_TEXT_ALIGN_LEFT);
                        }
                        else
                        {
                            result = field_draw_text(result, ot, CARD_MENU_TEXT(g_card_menu_location_names, preview->track.bits.music_track), FIELD_TEXT_COLOR_NORMAL,
                                                   x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, FIELD_TEXT_ALIGN_LEFT);
                        }
                    }
                }
                else
                {
                    result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_niki_text_wrong_version, CARD_MENU_TEXT_WRONG_VERSION), FIELD_TEXT_COLOR_NORMAL, -x_offset, -y_offset, FIELD_TEXT_ALIGN_LEFT);
                }
            }
            else
            {
                s32 slot_index;
                SaveFileHeader* header;

                terminate_multibyte_text(g_niki_entry_file.header.title);
                header = &g_niki_entry_file.header;
                if ((u32)(header->title[1][0] - 1) >= 0x7FU)
                {
                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = *(header->title[0] + slot_index);
                    }
                    name[slot_index] = 0;
                    result = draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = g_niki_entry_file.header.title[1][slot_index];
                    }
                    name[slot_index] = 0;
                    result = draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

#include "../../common/save_file/terminate_multibyte_text.inc.c"

#include "../../common/card_menu/card_menu_draw_cant_hold_more.inc.c"

#include "../../common/card_menu/card_menu_clear_elements.inc.c"

#include "../../common/card_menu/card_menu_alloc_element.inc.c"

/**
 * @brief Animate element windows and append their content and borders to the frame.
 * @param frame_arg Draw context supplying the clip variant and primitive cursor.
 */
void card_menu_update_and_draw_elements(FieldRenderHalf* frame_arg)
{
    void* prim;
    FieldRenderHalf* frame;
    CardMenuElement* element;
    s32 inset_width;
    s32 inset_height;
    s32 element_index;
    s32 entry_count;
    s32 opening_width_low;
    s32 closing_width_low;
    s32 opening_border_x;
    s32 opening_border_width_low;
    s32 closing_border_x;
    s32 closing_border_width_low;
    s32 open_border_width_low;
    DRAWENV draw_area;

    prim = frame_arg->primitive_cursor;
    frame = frame_arg;

    if (frame_arg->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_area, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }
    else
    {
        SetDefDrawEnv(&draw_area, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }

    element = g_card_menu_element_pool;
    for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++, element++)
    {
        if (element->attr.bits.state != CARD_MENU_ELEMENT_FREE)
        {
            entry_count = g_card_entry_state;
            if ((entry_count < CARD_MENU_ENTRY_COUNT_LIMIT) && (element->draw == card_menu_draw_entry_list) && (g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN))
            {
                if (entry_count * CARD_MENU_ENTRY_ROW_HEIGHT > g_card_menu_scroll_y + CARD_MENU_LIST_HEIGHT)
                {
                    prim = field_draw_menu_scroll_arrow(prim, frame->ordering_table, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
                }
                if (g_card_menu_scroll_y != 0)
                {
                    prim = field_draw_menu_scroll_arrow(prim, frame->ordering_table, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
                }
            }

            SetDrawEnv((DR_ENV*)prim, &draw_area);
            addPrim(frame->ordering_table, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.bits.state)
            {
            case CARD_MENU_ELEMENT_OPENING:
                g_pad_input = 0;
                opening_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                inset_width = (CARD_MENU_ELEMENT_WIDTH(element, opening_width_low) * element->attr.bits.transition_step) / 8;
                inset_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                prim = element->draw(frame->ordering_table, prim, (CARD_MENU_ELEMENT_WIDTH(element, opening_width_low) - inset_width) / 2,
                                                        (element->size.bits.height - inset_height) / 2);
                opening_border_x = element->attr.bits.x;
                opening_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim = field_draw_menu_frame(prim, frame->ordering_table,
                                                        opening_border_x + (CARD_MENU_ELEMENT_WIDTH(element, opening_border_width_low) - inset_width) / 2,
                                                        element->attr.bits.y + (element->size.bits.height - inset_height) / 2, inset_width, inset_height,
                                                        frame_arg->display_rect.y, element_index == 0);
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == 8)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = 2;
                }
                break;

            case CARD_MENU_ELEMENT_OPEN:
                prim = element->draw(frame->ordering_table, prim, 0, 0);
                open_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim =
                    field_draw_menu_frame(prim, frame->ordering_table, element->attr.bits.x, element->attr.bits.y, CARD_MENU_ELEMENT_WIDTH(element, open_border_width_low),
                                                     element->size.bits.height, frame_arg->display_rect.y, element_index == 0);
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSING:
                g_pad_input = 0;
                closing_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                inset_width = (CARD_MENU_ELEMENT_WIDTH(element, closing_width_low) * element->attr.bits.transition_step) / 8;
                inset_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                prim = element->draw(frame->ordering_table, prim, (CARD_MENU_ELEMENT_WIDTH(element, closing_width_low) - inset_width) / 2,
                                                        (element->size.bits.height - inset_height) / 2);
                closing_border_x = element->attr.bits.x;
                closing_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim = field_draw_menu_frame(prim, frame->ordering_table,
                                                        closing_border_x + (CARD_MENU_ELEMENT_WIDTH(element, closing_border_width_low) - inset_width) / 2,
                                                        element->attr.bits.y + (element->size.bits.height - inset_height) / 2, inset_width, inset_height,
                                                        frame_arg->display_rect.y, element_index == 0);
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = 3;
                    element->attr.bits.state = 4;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = 0;
                }
                break;
            }
        }
    }

    frame_arg->primitive_cursor = prim;
}

#include "../../common/card_menu/card_menu_deactivate_primary_element.inc.c"

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"

#include "../../common/card_menu/card_menu_draw_load_prompt.inc.c"

/**
 * @brief Draw loading progress, then restore a validated save and close the menu.
 *
 * A failed validation opens the status dialog. Once loading finishes successfully,
 * restore the saved fields and trailing data, then start every active window's
 * closing animation before returning to the game.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every caption x).
 * @param y_offset Vertical scroll offset (subtracted from every caption y).
 * @return Advanced primitive-buffer write cursor.
 */
void* card_menu_draw_transfer_window(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u16* text_table;
    NikiSaveBuffer* resource;
    CardMenuElement* element;
    void* result;
    s32 x;

    x = -x_offset + 0x90;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_niki_text_loading, CARD_MENU_TEXT_LOADING);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = card_menu_draw_progress_bar(result, ot);

    if (g_card_menu_progress_active == 0)
    {
        resource = &g_niki_save_blob;
        element = g_card_menu_element_pool;
        element->attr.bits.state = 0;
        if (validate_save_file(&resource->save) == 0)
        {
            card_menu_open_status_dialog(4);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        g_field_niki_state = 1;
        D_801227CC = resource->loaded.unknown_0x254;
        D_801227F4 = resource->loaded.unknown_0x256;
        D_8011F418 = g_card_slot;
        strcpy(D_8011F3D8, g_card_selected_save_path);
        bcopy(resource->loaded.trailing_data, g_field_shared_items, sizeof(resource->loaded.trailing_data));
        card_menu_close_all_elements();
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
    }

    return result;
}

#include "../../common/card_menu/card_menu_draw_progress_bar.inc.c"

#include "../../common/card_menu/card_menu_open_status_dialog.inc.c"

/**
 * @brief Open the secondary status window and reset the active card operation.
 * @param dialog_state Message selector consumed by the status draw callback.
 */
void niki_open_secondary_status_dialog(s32 dialog_state)
{
    CardMenuElement* element;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    element = &g_card_menu_element_pool[1];
    element->draw = card_menu_draw_save_status_dialog;
    element->attr.bits.transition_step = 1;
    element->attr.bits.state = 1;
    element->attr.bits.x = CARD_MENU_DIALOG_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    element->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_field_niki_state = 2;
    g_card_menu_write_in_progress = 0;
    g_card_menu_progress_active = 0;
    g_card_menu_selection_status = 0;
    g_card_menu_io_busy = 0;
    card_reset_entry_ranks();
    g_card_step = 0;
    g_card_menu_dialog_state = dialog_state;
}

#include "../../common/card_menu/card_menu_draw_status_dialog.inc.c"

#include "../../common/card_menu/card_menu_draw_save_status_dialog.inc.c"

#include "../../common/card_menu/card_menu_draw_icon_highlight.inc.c"

#include "../../common/card_menu/card_menu_enable_choice_toggle.inc.c"

#include "../../common/card_menu/card_menu_draw_choice_prompt.inc.c"

/**
 * @brief Access the extended metadata in the preview transfer buffer.
 * @return Preview metadata record.
 */
static inline SavedGameLayout* niki_preview_metadata(void)
{
    return &g_niki_entry_file.saved_game;
}

/**
 * @brief Draw the save-menu page and process its confirmation and progress states.
 * @param ot Ordering-table entry receiving the page primitives.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal displacement subtracted from glyph positions.
 * @param y_offset Vertical displacement subtracted from glyph positions.
 * @return Advanced GPU packet cursor.
 */
void* niki_draw_state_page(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 dispatch;
    dispatch = g_card_entry_state;
    switch (dispatch)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 x;
        u16* text_table;
        x = -x_offset + 0x90;
        text_table = &g_card_menu_text_table;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_card, CARD_MENU_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_EXCHANGE_STATE_NO_LOAD_FILE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_load_file, CARD_MENU_TEXT_NO_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL, -x_offset + 0x90, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_EXCHANGE_STATE_LOAD_PROGRESS:
    {
        s32 x;
        u16* text_table;
        s32 dialog_state;

        x = -x_offset + 0x90;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_niki_text_loading, CARD_MENU_TEXT_LOADING);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);

        prim = card_menu_draw_progress_bar(prim, ot);

        if (g_card_menu_progress_active == 0)
        {
            if (validate_save_file(&g_niki_save_blob.save) == 0)
            {
                card_menu_open_status_dialog(CARD_MENU_DIALOG_INVALID_SAVE);
                return prim;
            }
            field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
            g_card_entry_state = CARD_MENU_EXCHANGE_STATE_SAVE_CONFIRM;
            card_menu_enable_choice_toggle();
            field_reset_input_repeat();
        }
    }
    break;
    case CARD_MENU_EXCHANGE_STATE_CONFIRM_NO_SAVE:
    {
        s32 x;
        s32 y;
        CardMenuElement* packet;

        x = -x_offset;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_trade_data_not_saved, CARD_MENU_TEXT_TRADE_DATA_NOT_SAVED), FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        y = CARD_MENU_LINE_HEIGHT - y_offset;
        prim = card_menu_draw_choice_prompt(prim, ot, x + CARD_MENU_MESSAGE_WIDTH / 2, y);

        if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
        {
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            card_menu_enable_choice_toggle();
            g_card_entry_state = CARD_MENU_EXCHANGE_STATE_SAVE_CONFIRM;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_card_menu_choice_toggle != 0)
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                card_menu_enable_choice_toggle();
                g_card_entry_state = CARD_MENU_EXCHANGE_STATE_SAVE_CONFIRM;
                field_reset_input_repeat();
            }
            else
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                packet = g_card_menu_element_pool;
                g_field_niki_state = 2;
                card_menu_clear_elements();
                field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
                field_reset_input_repeat();
            }
        }
    }
    break;
    case CARD_MENU_EXCHANGE_STATE_SAVE_CONFIRM:
    {
        s32 x;
        s32 y;
        u16* text_table;
        s32 record_count;
        s32 record_index;
        u8(*records)[64];
        NikiSaveBuffer* resource;
        s32 checksum;

        x = -x_offset;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_found_load_file, CARD_MENU_TEXT_FOUND_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_niki_text_found_load_file, CARD_MENU_TEXT_FOUND_LOAD_FILE);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_OVERWRITE_TRADE_DATA), FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_MESSAGE_WIDTH / 2, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);

        y = CARD_MENU_LINE_HEIGHT * 2 - y_offset;
        prim = card_menu_draw_choice_prompt(prim, ot, x + CARD_MENU_MESSAGE_WIDTH / 2, y);

        if ((g_pad_input & NIKI_CANCEL_INPUT_MASK) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && g_card_menu_choice_toggle != 0))
        {
            card_menu_enable_choice_toggle();
            g_card_entry_state = CARD_MENU_EXCHANGE_STATE_CONFIRM_NO_SAVE;
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            records = (u8(*)[64])g_field_shared_items;
            resource = &g_niki_save_blob;
            bcopy(g_field_shared_items, resource->loaded.trailing_data, sizeof(resource->loaded.trailing_data));
            record_count = 0;
            for (record_index = 0; record_index < 4; record_index++)
            {
                if (records[record_index][0] != 0)
                {
                    record_count++;
                }
            }
            resource->loaded.trailing_record_count = record_count;
            checksum = compute_save_checksum(&resource->save);
            resource->save.magic = SAVE_FILE_MAGIC;
            resource->save.checksum = checksum;
            g_card_menu_write_in_progress = 1;
            g_card_step = g_card_steps_write_save;
            g_card_entry_state = CARD_MENU_EXCHANGE_STATE_SAVE_PROGRESS;
        }
    }
    break;
    case CARD_MENU_EXCHANGE_STATE_SAVE_PROGRESS:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + 0x90;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_niki_text_saving, CARD_MENU_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_niki_text_saving, CARD_MENU_TEXT_SAVING);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);

        prim = card_menu_draw_progress_bar(prim, ot);

        if (g_card_menu_write_in_progress == 0)
        {
            field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
            card_menu_clear_elements();
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
            g_field_niki_state = 0;
        }
    }
    break;
    default:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + 0x90;
        text_table = &g_card_menu_text_table;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);

        if (g_card_menu_entry_scan_active == 0)
        {
            if (g_card_menu_io_busy != 0)
            {
                return prim;
            }
            if ((u32)(*g_card_step - 6) < 2U)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, 0xC) != 0) ||
                (niki_preview_metadata()->identity.ids.game_id != D_801227CC) || (niki_preview_metadata()->identity.ids.save_id != D_801227F4))
            {
                g_card_menu_selected_row++;
                if (g_card_menu_selected_row >= g_card_entry_state)
                {
                    if (g_card_entry_state != 0)
                    {
                        g_card_entry_state = CARD_MENU_EXCHANGE_STATE_NO_LOAD_FILE;
                    }
                    else
                    {
                        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                    }
                }
                else
                {
                    card_menu_commit_selected_entry();
                    card_menu_scroll_to_selection();
                }
            }
            else
            {
                g_card_menu_progress_start_tick = VSync(-1);
                g_card_menu_progress_active = 1;
                g_card_step = g_card_steps_read_saved_copy;
                g_card_entry_state = CARD_MENU_EXCHANGE_STATE_LOAD_PROGRESS;
            }
        }
    }
    break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    }

    if (g_card_menu_io_busy != 0)
    {
        return prim;
    }
    if (g_card_entry_state == CARD_MENU_EXCHANGE_STATE_LOAD_PROGRESS)
    {
        return prim;
    }
    if (g_card_entry_state == CARD_MENU_EXCHANGE_STATE_SAVE_PROGRESS)
    {
        return prim;
    }
    if (g_card_entry_state == CARD_MENU_EXCHANGE_STATE_SAVE_CONFIRM)
    {
        return prim;
    }
    if (g_card_entry_state == CARD_MENU_EXCHANGE_STATE_CONFIRM_NO_SAVE)
    {
        return prim;
    }

    if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
    {
        g_field_niki_addhero_state = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        card_menu_close_all_elements();
        return prim;
    }

    if ((g_pad_input & CARD_MENU_CARD_SWITCH_BUTTON_MASK) && (g_card_entry_state != CARD_MENU_ENTRY_STATE_CHECKING_CARD))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        D_80164B80 = 0;
        g_card_step = 0;
        g_card_menu_scroll_frames = 0;
        g_card_menu_scroll_target_y = 0;
        g_card_menu_scroll_y = 0;
        g_card_menu_selected_row = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_card_menu_selection_status = 0;
        g_card_slot ^= 1;
        card_reset_entry_ranks();
        g_card_menu_progress_bar_active = 0;
        g_card_step = g_card_steps_initial_scan;
    }

    return prim;
}

#include "../../common/save_file/skip_hex_digits.inc.c"

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"
