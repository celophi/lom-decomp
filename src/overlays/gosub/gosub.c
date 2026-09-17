#include "gosub_internal.h"

/* Overlay BSS layout is address-sensitive; do not reorder these definitions. */

s32 g_gosub_frame_parity;
s32 g_gosub_finished;
s32 g_gosub_cursor_row;
s32 g_gosub_row_count;
s32 g_gosub_visible_row_count;
/** @brief Screen-sequence cursor stored in a four-byte BSS slot. */
u8 g_gosub_screen_sequence_index;
u8 g_gosub_screen_sequence_index_storage[4] __asm__("g_gosub_screen_sequence_index");
s32 g_gosub_scroll_frames_remaining;
s32 g_gosub_combination_variant;
s32 g_gosub_dialog_choice;
s32 g_gosub_combination_quantity;
s32 g_gosub_allow_duplicate_selection;
/** @brief Encoded text currently displayed by the modal message dialog. */
u8* g_gosub_dialog_text;
s32 (*g_gosub_finish_handler)(void);
u8 g_gosub_selection_mode;
/** @brief Required selection count stored in a three-byte BSS slot. */
u8 g_gosub_required_selection_count;
u8 g_gosub_required_selection_count_storage[3] __asm__("g_gosub_required_selection_count");
/** @brief Row-detail flag stored in an eight-byte BSS slot. */
s32 g_gosub_show_row_details;
u8 g_gosub_show_row_details_storage[8] __asm__("g_gosub_show_row_details");
s32 g_gosub_result_rows[16];
s32 g_gosub_dialog_accepting_input;
u8 g_gosub_selected_rows[4];
s32 g_gosub_window_height;
s32 (*g_gosub_select_handler)(void);
s32 g_gosub_window_width;
s32 g_gosub_suppress_dialog_sound;
u8 g_gosub_text_buffers[0x5000];
/** @brief Current selection count stored in an eight-byte BSS slot. */
u8 g_gosub_selection_count;
u8 g_gosub_selection_count_storage[8] __asm__("g_gosub_selection_count");
u8 g_gosub_screen_sequence[20];
s32 g_gosub_combination_result_id;
/** @brief List row height stored in an eight-byte BSS slot. */
s32 g_gosub_row_height;
u8 g_gosub_row_height_storage[8] __asm__("g_gosub_row_height");
s32 g_gosub_scroll_y;
/** @brief Whether the next confirmed sort uses ascending order. */
s32 g_gosub_sort_ascending;
s32 g_gosub_scroll_target_y;
u8* g_gosub_title_text;
GosubElement g_gosub_elements[1];
GosubElement g_gosub_dynamic_elements[GOSUB_ELEMENT_COUNT - 1];
GosubListRow g_gosub_rows[512];
s32 (*g_gosub_dialog_handler)(s32);

/**
 * @brief Open the gosub overlay for a sequence of screen ids.
 *
 * @param unused Unused loader argument.
 * @param screen_sequence s32 array terminated by
 *        @c GOSUB_SCREEN_SEQUENCE_END.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/qM81L
 */
void gosub_open_screen_sequence(void* unused, s32* screen_sequence)
{
    field_set_default_fade_target();
    g_gosub_frame_parity = 0;
    g_gosub_finished = 0;
    func_800AA02C();
    gosub_load_screen_sequence(screen_sequence);
}

/**
 * @brief Run one frame of the gosub overlay and return its completion state.
 * @param render_context Active field rendering context.
 * @return Nonzero after the current gosub sequence finishes.
 * @see decomp.me (100%) https://decomp.me/scratch/ykfW4
 */
s32 gosub_update_frame(GosubRenderContext* render_context)
{
    s32* frame_parity;
    s32 finished;
    field_text_reset_scratch();
    gosub_update_screen(render_context);
    func_80063194();
    frame_parity = &g_gosub_frame_parity;
    finished = g_gosub_finished;
    *frame_parity ^= 1;
    return finished;
}

/**
 * @brief Copy and enter a GOSUB_SCREEN_SEQUENCE_END-terminated screen sequence.
 * @param screen_sequence Sequence of screen ids stored as s32 values.
 * @see decomp.me (100%) https://decomp.me/scratch/weBhP
 */
void gosub_load_screen_sequence(s32* screen_sequence)
{
    u8* sequence_cursor;
    s32 screen_count;
    u8 screen_id;
    s32 stack_pad[2];

    gosub_upload_ui_image();
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    gosub_clear_elements();
    g_gosub_sort_ascending = 0;
    g_gosub_screen_sequence_index = 0;
    g_gosub_result_count = 0;
    g_gosub_dialog_handler = gosub_handle_backtrack_dialog;
    screen_count = 0;
    if (*screen_sequence != GOSUB_SCREEN_SEQUENCE_END)
    {
        u8* sequence = g_gosub_screen_sequence;
        s32 sentinel = GOSUB_SCREEN_SEQUENCE_END;
        sequence_cursor = (u8*)screen_sequence;
        do
        {
            screen_id = *sequence_cursor;
            sequence_cursor += 4;
            *((u8*)(screen_count + (u32)sequence)) = screen_id;
            screen_count++;
        } while (*(s32*)sequence_cursor != sentinel);
    }
    g_gosub_screen_sequence[screen_count] = ((u8*)screen_sequence)[screen_count * 4];
    g_gosub_dialog_accepting_input = 0;
    gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
    gosub_upload_font_texture();
}

/**
 * @brief Initialize a gosub sub-screen and install its selection callbacks.
 * @param screen_id Screen id, 0..19; anything else returns without touching state.
 * @see decomp.me (100%)
 */
void gosub_enter_screen(s32 screen_id)
{
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    g_gosub_combination_variant = 0;
    g_gosub_combination_result_id = 0;
    g_gosub_combination_quantity = 0;
    g_gosub_allow_duplicate_selection = 0;
    g_gosub_show_row_details = 0;

    switch (screen_id)
    {
    case 0:
        gosub_start_element_exit();
        gosub_build_screen_0_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(8);
        }
        break;

    case 1:
        gosub_start_element_exit();
        gosub_build_screen_1_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0xA);
        }
        break;

    case 2:
        gosub_start_element_exit();
        gosub_build_equipment_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(2);
        }
        break;

    case 3:
        gosub_start_element_exit();
        gosub_build_equipment_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(4);
        }
        break;

    case 4:
        gosub_start_element_exit();
        gosub_build_equipment_list(2);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(6);
        }
        break;

    case 5:
        gosub_start_element_exit();
        gosub_build_equipment_list(3);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0);
        }
        break;

    case 6:
    case 7:
    case 8:
        gosub_start_element_exit();
        gosub_build_grouped_option_list(screen_id - 6);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        break;

    case 9:
        gosub_start_element_exit();
        gosub_build_equipment_list(4);
        g_gosub_required_selection_count = 4;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_update_group_selection;
        g_gosub_finish_handler = gosub_publish_group_selection;
        gosub_build_screen_9_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-2);
        }
        break;

    case 10:
        gosub_start_element_exit();
        gosub_build_equipment_list(3);
        g_gosub_show_row_details = 1;
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        g_gosub_combination_variant = 0;
        g_gosub_combination_result_id = 0;
        g_gosub_combination_quantity = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = gosub_validate_pending_pair_selection;
        g_gosub_finish_handler = gosub_publish_two_row_selection;
        g_gosub_dialog_handler = gosub_handle_combination_dialog;
        gosub_build_screen_10_elements();
        if (g_pad_ctx[0x29D6] >= 0x28)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-4);
        }
        break;

    case 11:
        gosub_start_element_exit();
        gosub_build_packed_record_list();
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = gosub_commit_row_reorder;
        g_gosub_finish_handler = gosub_publish_selection;
        g_gosub_allow_duplicate_selection = 1;
        gosub_build_screen_11_elements();
        if (g_pad_ctx[0x29D6] == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-6);
        }
        break;

    case 12:
        gosub_start_element_exit();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row_with_validation;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x32);
        }
        break;

    case 13:
        gosub_start_element_exit();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row_with_validation;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x34);
        }
        break;

    case 14:
        gosub_start_element_exit();
        gosub_build_roster_list(2);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x36);
        }
        break;

    case 15:
        gosub_start_element_exit();
        gosub_build_screen_15_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x3E);
        }
        break;

    case 16:
        gosub_start_element_exit();
        gosub_build_screen_16_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(0);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x3C);
        }
        break;

    case 17:
        gosub_start_element_exit();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x32);
        }
        break;

    case 18:
        gosub_start_element_exit();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x34);
        }
        break;

    case 19:
        gosub_start_element_exit();
        gosub_build_screen_19_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x4C);
        }
        break;
    }
}
