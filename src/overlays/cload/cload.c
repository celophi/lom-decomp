#include "internal/cload_internal.h"
#include "overlays/field/field_text.h"
#include "main/display.h"
#include <libetc.h>
#include "main/controller.h"
#include "main/cdrom.h"
#include "overlays/field/field_fade.h"
#include "overlays/field/field_input.h"

void reset_controller_vsync_state(void);
void set_controller_vsync_interval(u32 interval);
void cload_init_card_events(void);

/**
 * @brief Left edges of the two card slot label windows.
 * @note JP moves both right by 8.
 */
#if defined(VERSION_JP)
#define CLOAD_CARD_SLOT0_LABEL_X 32
#define CLOAD_CARD_SLOT1_LABEL_X 176
#else
#define CLOAD_CARD_SLOT0_LABEL_X 24
#define CLOAD_CARD_SLOT1_LABEL_X 168
#endif

/**
 * @brief Entry list column of the rank marker.
 * @note JP moves it right.
 */
#if defined(VERSION_JP)
#define CLOAD_ENTRY_MARKER_X 0xCC
#else
#define CLOAD_ENTRY_MARKER_X 0xC2
#endif

/**
 * @brief Initialize and run the CLOAD save/continue menu.
 * @return CLOAD result code set by the menu loop.
 */
s32 cload_main(void)
{
    RECT rect;

    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;
    card_reset_entry_ranks();
    cload_load_icon_resources();
    cload_init_display();
    g_cload_result = 0;
    cload_init_card_events();
    g_cload_icon_phase = 0;
    setRECT(&rect, 0x140, 0, 0x40, 0x100);
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();
    D_80162370 = 0;
    g_cload_progress_active = 0;
    g_cload_selection_status = 0;
    g_cload_io_busy = 0;
    g_card_menu_frame_parity = 0;
    g_card_menu_exit_requested = 0;
    field_reset_input_repeat();
    cload_build_ui_elements();
    cload_run_menu_loop();
    return g_cload_result;
}

/**
 * @brief Run the double-buffered CLOAD menu loop until it exits.
 */
void cload_run_menu_loop(void)
{
    RECT rect;
    FieldRenderHalf *frame;
    u_long *ordering_table;
    s32 buffer_index;
    s32 dpad_input;

    DrawSync(0);
    VSync(0);
    setRECT(&rect, 0, 0, 0x140, 0x1D8);
    ClearImage(&rect, 0, 0, 0);
    frame = &g_cload_render_buffers[0];
    buffer_index = 0;
    ClearOTagR(&frame->ordering_table[FIELD_FADE_OT_INDEX], CLOAD_OT_SIZE);
    ClearOTagR(&g_cload_render_buffers[1].ordering_table[FIELD_FADE_OT_INDEX], CLOAD_OT_SIZE);
    PutDispEnv(&frame->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ordering_table = &frame->ordering_table[FIELD_FADE_OT_INDEX];
        ClearOTagR(ordering_table, CLOAD_OT_SIZE);
        frame->primitive_cursor = g_cload_primitive_buffers[buffer_index];
        field_update_input_repeat();
        dpad_input = g_pad_input & 0xF000;
        if (dpad_input != 0)
        {
            g_pad_input = dpad_input;
        }
        field_update_and_render_fade(frame);
        if (card_menu_update_frame(frame) != 0)
        {
            break;
        }
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(&frame->display_rect, 0, 0, 0);
        buffer_index = 0;
        if (frame == &g_cload_render_buffers[0])
        {
            frame = &g_cload_render_buffers[1];
            buffer_index = 1;
        }
        else
        {
            frame = &g_cload_render_buffers[0];
        }
        PutDispEnv(&frame->disp_env);
        PutDrawEnv(&frame->draw_env);
        DrawOTag(ordering_table + CLOAD_OT_SIZE - 1);
        update_controllers();
        cdrom_process_state();
    } while (1);
    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Initialize the CLOAD display and draw buffers.
 */
void cload_init_display(void)
{
    s32 stack_frame_pad[2];
    SetGeomScreen(SCREEN_PROJECTION_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    setRECT(&g_cload_render_buffers[0].display_rect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    setRECT(&g_cload_render_buffers[1].display_rect, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    DrawSync(0);
    VSync(0);
    SetDefDispEnv(&g_cload_render_buffers[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_cload_render_buffers[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_cload_render_buffers[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_cload_render_buffers[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    g_cload_render_buffers[1].draw_env.dtd = 0;
    g_cload_render_buffers[0].draw_env.dtd = 0;
    field_reset_fade_state();
    field_set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, 0x14);
}

#include "../../common/card_menu/card_menu_update_frame.inc.c"

/**
 * @brief Allocate and lay out the five fixed windows of the load screen.
 */
void cload_build_ui_elements(void)
{
    CardMenuElement *element;
    s32 unused[2];

    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_cload_selection_status = 0;
    cload_clear_elements();
    /* Hold slot 0 so the fixed elements below are allocated from slot 1 on. */
    g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_OPENING;

    element = cload_alloc_element();
    element->draw = cload_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 28;
    element->attr.bits.y = 74;
    element->size.bits.flag = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, 8);
    element->size.bits.width_high = 1;
    CLOAD_SET_ELEMENT_HEIGHT(element, 73);

    element = cload_alloc_element();
    element->draw = cload_draw_header_label;
    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 80;
    element->attr.bits.y = 12;
    element->size.bits.flag = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, 0xA0);
    element->size.bits.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CLOAD_CARD_SLOT0_LABEL_X;
    element->draw = cload_draw_card_slot0_label;
    element->attr.bits.y = 44;
    element->size.bits.flag = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);
    element->size.bits.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CLOAD_CARD_SLOT1_LABEL_X;
    element->draw = cload_draw_card_slot1_label;
    element->attr.bits.y = 44;
    element->size.bits.flag = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);
    element->size.bits.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->draw = cload_draw_selected_entry_details;
    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 30;
    element->attr.bits.y = 160;
    element->size.bits.flag = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, 4);
    element->size.bits.width_high = 1;
    CLOAD_SET_ELEMENT_HEIGHT(element, 51);

    g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

/**
 * @brief Update input, loading state, scrolling, and UI elements for one frame.
 * @param frame Render buffer being built this frame.
 */
void card_menu_update_state(FieldRenderHalf *frame)
{
    s32 delta;

    card_menu_update_elements(frame);
    g_cload_icon_phase += 2;
    if ((g_cload_element1_state & 0x7F) == 2)
    {
        cload_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    cload_handle_input();
    if (g_card_menu_scroll_frames != 0)
    {
        s32 base = g_card_menu_scroll_y;
        delta = (g_card_menu_scroll_target_y - g_card_menu_scroll_y) / g_card_menu_scroll_frames;
        g_card_menu_scroll_frames -= 1;
        g_card_menu_scroll_y += delta;
    }
    else
    {
        g_card_menu_scroll_y = g_card_menu_scroll_target_y;
    }
}

/**
 * @brief Advance the active load sequence and react to its phase result.
 */
void cload_update_load_sequence(void)
{
    s32 phase;

    if (g_card_entry_state >= 0x10)
    {
        if (g_cload_load_step == NULL)
        {
            g_cload_load_step = g_cload_steps_initial_scan;
        }
    }
    do
    {
        phase = cload_advance_load_sequence();
    } while (phase == 3);
    if (phase == 2)
    {
        g_cload_load_step = g_cload_steps_card_reset;
    }
    if (phase == 4)
    {
        g_cload_load_step = g_cload_steps_refresh_entries;
    }
    if (phase == 5)
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
        g_cload_load_step = g_cload_steps_card_reset;
    }
}

/**
 * @brief Handle CLOAD menu navigation, confirm, and cancel input.
 * @return Input-handler status used by the caller.
 */
s32 cload_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 sfx_id;
    CardMenuElement *prompt;

    if (g_card_menu_element_pool[1].attr.bits.state == CARD_MENU_ELEMENT_FREE)
    {
        g_card_menu_exit_requested = 1;
        return;
    }
    if (g_card_menu_exit_requested != 0)
    {
        return;
    }
    if (g_card_menu_element_pool[1].attr.bits.state >= CARD_MENU_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_card_menu_element_pool[0].attr.bits.state != CARD_MENU_ELEMENT_FREE)
    {
        return;
    }
    pending = g_card_entry_state;
    if (pending == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
    {
        return;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return;
    }
    if (g_cload_io_busy != 0)
    {
        return;
    }
    if (*g_cload_load_step >= 6 && *g_cload_load_step <= 7)
    {
        return;
    }
    status = g_pad_input;
    if (status & 0x40)
    {
        g_card_menu_exit_requested = 1;
        g_cload_result = 1;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        return;
    }
    if (status & CARD_MENU_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_card_menu_scroll_frames = 0;
        g_card_menu_scroll_target_y = 0;
        g_card_menu_scroll_y = 0;
        g_card_menu_selected_row = 0;
        g_cload_load_step = NULL;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_cload_selection_status = 0;
        g_card_slot ^= 1;
        card_reset_entry_ranks();
        return;
    }
    if (pending >= 0x10)
    {
        return;
    }
    count = 1;
    if (status & 8)
    {
        g_pad_input = 0x4000;
        count = 1;
    }
    if (g_pad_input & 4)
    {
        g_pad_input = 0x1000;
        count = 1;
    }
    while (count != 0)
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
        count -= 1;
    }
    if (g_pad_input & 0x5000)
    {
        cload_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        cload_scroll_to_selection();
        return;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, 0xC) != 0)
        {
            sfx_id = 0x78;
        }
        else
        {
            if ((g_cload_selected_file.saved_game.compatibility_tag == g_save_compatibility_tag) ||
                (g_cload_selected_file.saved_game.compatibility_tag == SAVE_TAG_ANY))
            {
                prompt = cload_alloc_element();
                prompt->draw = cload_draw_load_prompt;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = 16;
                prompt->attr.bits.y = 91;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = 43;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
                cload_enable_choice_toggle();
                cload_restart_load_sequence();
                sfx_id = 0x7E;
            }
            else
            {
                sfx_id = 0x78;
            }
        }
        field_play_sound(sfx_id, 0x80);
    }
}

/**
 * @brief Put every live UI element into its closing state.
 */
void cload_close_all_elements(void)
{
    CardMenuElement *element;
    s32 i;

    element = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != CARD_MENU_ELEMENT_FREE)
        {
            element->attr.bits.state = CARD_MENU_ELEMENT_CLOSING;
            element->attr.bits.transition_step = CARD_MENU_ELEMENT_TRANSITION_STEPS;
        }
    }
}

/**
 * @brief Move the list scroll target to keep the selected row visible.
 */
void cload_scroll_to_selection(void)
{
    s32 base;
    s32 delta;

    base = g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
    delta = base - g_card_menu_scroll_y;
    if (delta >= 0x3C)
    {
        g_card_menu_scroll_target_y = base - 0x38;
        g_card_menu_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
    if (delta < 0)
    {
        g_card_menu_scroll_target_y = g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
        g_card_menu_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
}

#include "../../common/card_menu/card_menu_update_elements.inc.c"

/**
 * @brief Draw the visible save-entry list and selection cursor.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void *cload_draw_entry_list(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_card_entry_state;

    switch (state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        do
        {
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_no_lom_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 1, -x_offset + 0x84, -y_offset, 2);
        } while (0);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_no_lom_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_no_memory_card, CARD_MENU_TEXT_NO_CARD), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (g_cload_entry_scan_active != 0)
        {
            s32 x;
            u16 *text_table;
        case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
            x = -x_offset + 0x84;
            text_table = &g_cload_text_check_memory_card;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), 1, x, -y_offset, 2);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), 1, x, 0xE - y_offset, 2);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), 1, x, 0x1C - y_offset, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 off;
            s32 base_x;
            s32 *flag_ptr;
            u16 marker_offset;
            uintptr_t entry;
            Vec2s pos;
            u16 *text_table;

            off = i;
            base_x = -x_offset;
            text_table = &g_cload_text_check_memory_card;
            entry = (uintptr_t)g_card_entries;
            off = i;
            do
            {
                row_y = ((i * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_card_menu_scroll_y;
                if (row_y >= -13 && row_y <= 72)
                {
                    flag_ptr = (s32 *)((u8 *)g_card_entry_ranks + off);
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + CARD_MENU_ENTRY_VALUE_X;
                        pos.y = row_y;
                        prim = field_draw_text(field_draw_number(ot, prim, *(s32*)((u8*)g_card_entry_suffix_values + off), 1, &pos, 0), ot,
                                             CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_number_prefix), 1, base_x + 0x70, row_y, 0);
                        if ((g_card_rank_count - 1) == *flag_ptr)
                        {
                            marker_offset = text_table[27];
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), 1, base_x + CLOAD_ENTRY_MARKER_X, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            marker_offset = text_table[28];
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), 1, base_x + CLOAD_ENTRY_MARKER_X, row_y, 0);
                        }
                        if (*skip_hex_digits((u8*)((g_card_slot * CARD_DIRECTORY_BYTES) + entry + 0xC)) == 0x2B)
                        {
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_plus_marker), 1, 0xF8 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, (char*)((g_card_slot * CARD_DIRECTORY_BYTES) + entry), 0xC) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_mana), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, (char*)((g_card_slot * CARD_DIRECTORY_BYTES) + entry), 0xC) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_alt_save), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, (char*)((g_card_slot * CARD_DIRECTORY_BYTES) + entry), 8) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_new_save), 1, base_x, row_y, 0);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_cload_text_other_game), 1, base_x, row_y, 0);
                    }
                }
                entry += CARD_DIRECTORY_ENTRY_BYTES;
                off += 4;
                i++;
            } while (i < g_card_entry_state);
        }
            row_y = ((g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_card_menu_scroll_y;

            if (g_cload_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = CARD_MENU_HIGHLIGHT_COLOR;
                setlen(tile, 3);
                setcode(tile, 0x62);
                tile->y0 = (s16)(row_y - 1);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->h = 0xE;
                addPrim(ot, tile);
                prim = tile + 1;
            }
        }
        break;
    }
    return prim;
}

#include "../../common/save_file/skip_hex_digits.inc.c"

/**
 * @brief Draw the fixed CLOAD header label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void *cload_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_load, CARD_MENU_TEXT_LOAD_TITLE), 1, -x_offset + 0x50, -y_offset, 2);
}

/**
 * @brief Draw the first memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void *cload_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 color;
    u8 *text;
    RECT unused;

    color = 1;
    text = CARD_MENU_TEXT_AT(g_card_menu_text_card_slot0_label, CARD_MENU_TEXT_CARD_SLOT0_LABEL);
    if (g_card_slot != 0)
    {
        color = 3;
    }
    return field_draw_text(prim, ot, text, color, -x_offset + CARD_MENU_CARD_LABEL_TEXT_X, -y_offset, 2);
}

/**
 * @brief Draw the second memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void *cload_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 color;
    u8 *text;
    RECT unused;

    color = 1;
    text = CARD_MENU_TEXT_AT(g_card_menu_text_card_slot1_label, CARD_MENU_TEXT_CARD_SLOT1_LABEL);
    if (g_card_slot == 0)
    {
        color = 3;
    }
    return field_draw_text(prim, ot, text, color, -x_offset + CARD_MENU_CARD_LABEL_TEXT_X, -y_offset, 2);
}

/**
 * @brief Draw metadata for the selected save entry.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Draws party icons, play time, player name and location for compatible
 *       Legend of Mana saves. Other games use their memory-card title.
 *       The JP layout places the time and name columns farther left.
 */
void* cload_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    Vec2s pos;
    u8 name[0x100];
    s32 party_icon[3];

    result = prim;
    if (g_cload_selection_status == 0)
    {
        return result;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return result;
    }
    if (g_cload_selection_status == 3)
    {
        return result;
    }
    if (g_card_entry_state >= 0x10)
    {
        return result;
    }
    if (g_cload_selection_status == 2)
    {
        s32 x = -x_offset;
        u16* text_table;

        result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_new_save_prompt, CARD_MENU_TEXT_NEW_SAVE_TITLE), 1, x, -y_offset, 0);
        text_table = CARD_MENU_TEXT_TABLE(g_cload_text_new_save_prompt, 20);
        return field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_USES_TWO_BLOCKS), 1, x, 0x10 - y_offset, 0);
    }
    else
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, 0xC) == 0)
        {
            SavedGameLayout* save = &g_cload_selected_file.saved_game;

            if (save->compatibility_tag == SAVE_TAG_ANY || save->compatibility_tag == g_save_compatibility_tag)
            {
                s32 present_count;
                s32 i;
                s32 j;
                s32 step;
                s32 half_step;
                s32 base_x;
                s32 base_y;
                s32 total;
                s32 hours;

                total = 0;
                party_icon[0] = save->spawn.bits.party_icon_0;
                party_icon[1] = save->track.bits.party_icon_1;
                party_icon[2] = save->track.bits.party_icon_2;
                g_card_menu_icon_palette = save->icon_palette;

                present_count = 0;
                for (i = 0; i < 3; i++)
                {
                    if (party_icon[i] != SAVE_NO_ICON)
                    {
                        present_count += 1;
                    }
                }

                switch (present_count)
                {
                case 2:
                    step = 0x20;
                    half_step = 0x10;
                    g_cload_icon_phase %= 0x20;
                    break;
                case 3:
                    step = 0x10;
                    half_step = 0x20;
                    g_cload_icon_phase %= 0x60;
                    break;
                default:
                    step = 0x10;
                    half_step = 0x20;
                    g_cload_icon_phase = 0x1F;
                    break;
                }

                i = 0;
                j = i;
                for (; j < 3; j++)
                {
                    base_y = i * half_step;
                    base_x = base_y + half_step;
                    if (party_icon[j] != SAVE_NO_ICON)
                    {
                        s32 adjust = step;
                        s32 rem;
                        s32 hi;

                        if (g_cload_icon_phase >= base_y && g_cload_icon_phase < base_x)
                        {
                            adjust += g_cload_icon_phase - base_y;
                        }
                        else
                        {
                            rem = base_x % (half_step * present_count);
                            if (g_cload_icon_phase >= rem)
                            {
                                hi = rem + half_step;
                                if (g_cload_icon_phase < hi)
                                {
                                    adjust += hi - g_cload_icon_phase;
                                }
                            }
                        }
                        result = cload_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, party_icon[j], i, j);
                        total += adjust;
                        i += 1;
                    }
                }

                {
                    SavedGameLayout* shown_save = &g_cload_selected_file.saved_game;
                    s32 x = -x_offset;
                    s32 y = -y_offset;
                    s32 time_x = x + CARD_MENU_DETAILS_HOURS_RIGHT_X;

                    base_y = shown_save->play_time;

                    pos.x = time_x;
                    pos.y = (s16)y;
                    hours = base_y / 216000;
                    result = field_draw_number(ot, result, hours, 1, &pos, 1);
                    result = field_draw_text(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, 25), 1, x + CARD_MENU_DETAILS_TIME_SEPARATOR_X, y, 0);
                    base_y = (base_y / 3600) - (hours * 0x3C);
                    if (base_y < 0xA)
                    {
                        pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X);
                        pos.y = (s16)y;
                        result = field_draw_number(ot, result, 0, 1, &pos, 1);
                    }
                    pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_RIGHT_X);
                    pos.y = (s16)y;
                    result = field_draw_number(ot, result, base_y, 1, &pos, 1);
                    result = field_draw_text(result, ot, shown_save->summary_name, 1, x + CARD_MENU_DETAILS_TEXT_X, y + 0x10, 0);
                    result = field_draw_text(result, ot, CARD_MENU_TEXT(g_cload_location_names, shown_save->track.bits.music_track), 1, x + CARD_MENU_DETAILS_TEXT_X,
                                           y + 0x20, 0);
                }
            }
            else
            {
                result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_cload_text_version_error, CARD_MENU_TEXT_WRONG_VERSION), 1, -x_offset, -y_offset, 0);
            }
        }
        else
        {
            s32 j;

            {
                SaveFileHeader* header;
                terminate_multibyte_text(g_cload_selected_file.header.title);
                header = &g_cload_selected_file.header;
                if (header->title[1][0] != 0 && header->title[1][0] < 0x80)
                {
                    return result;
                }

                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = *(header->title[0] + j);
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset, 1, 0);

                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = g_cload_selected_file.header.title[1][j];
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 1, 0);
            }
        }
    }
    return result;
}

#include "../../common/save_file/terminate_multibyte_text.inc.c"

/**
 * @brief Mark all eight UI elements as inactive.
 */
void cload_clear_elements(void)
{
    CardMenuElement *p;
    s32 i;

    p = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++)
    {
        p->attr.bits.state = CARD_MENU_ELEMENT_FREE;
        p++;
    }
}

/**
 * @brief Activate and return the first free UI element.
 * @return First free element, or the pool head if all slots are busy.
 */
CardMenuElement *cload_alloc_element(void)
{
    CardMenuElement *p;
    s32 i;

    p = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == CARD_MENU_ELEMENT_FREE)
        {
            p->size.bits.flag = 1;
            p->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
            return p;
        }
    }
    return g_card_menu_element_pool;
}

/**
 * @brief Advance and draw the eight pool elements for one frame.
 *
 * Links a draw-environment packet for each live element into the ordering
 * table, then animates it by state: an opening window grows, an open window
 * holds, a closing window shrinks, and a closed window counts down to free.
 *
 * @param frame Render buffer being built; primitive_cursor is read on entry and written back on exit.
 */
void card_menu_update_and_draw_elements(FieldRenderHalf *frame)
{
    void *arrow_prim;
    void *prim;
    u_long *ot;
    CardMenuElement *element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    arrow_prim = frame->primitive_cursor;
    ot = &frame->ordering_table[FIELD_FADE_OT_INDEX];

    if ((g_card_entry_state < 0x10) && ((g_cload_element1_state & CARD_MENU_ELEMENT_STATE_MASK) == 2))
    {
        if ((g_card_entry_state * CARD_MENU_ENTRY_ROW_HEIGHT) > (g_card_menu_scroll_y + 0x49))
        {
            arrow_prim = cload_emit_scroll_arrow(arrow_prim, ot, 0x114, 0x87, 0);
        }
        if (g_card_menu_scroll_y != 0)
        {
            arrow_prim = cload_emit_scroll_arrow(arrow_prim, ot, 0x114, 0x4A, 1);
        }
    }

    if (frame->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, 8, 0x140, 0xE0);
    }

    prim = arrow_prim;
    element = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != 0)
        {
            SetDrawEnv((DR_ENV *)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV *)prim + 1;

            switch (element->attr.bits.state)
            {
            case CARD_MENU_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARD_MENU_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / 8;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                   element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                   frame->display_rect.y, element->size.bits.flag);
                }
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == CARD_MENU_ELEMENT_TRANSITION_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
                }
                break;

            case CARD_MENU_ELEMENT_OPEN:
                prim = element->draw(ot, prim, 0, 0);
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, element->attr.bits.x, element->attr.bits.y, CARD_MENU_ELEMENT_WIDTH(element, width_low),
                                                   element->size.bits.height, frame->display_rect.y, element->size.bits.flag);
                }
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARD_MENU_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / 8;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                   element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                   frame->display_rect.y, element->size.bits.flag);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = 3;
                    element->attr.bits.state = CARD_MENU_ELEMENT_CLOSED;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = CARD_MENU_ELEMENT_FREE;
                }
                break;
            }
        }
    }

    frame->primitive_cursor = (u8 *)cload_emit_icon_highlight_strip(prim, ot);
}

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"
