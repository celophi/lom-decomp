#include "field_text.h"
#include "gosub_internal.h"

/* Overlay BSS layout is address-sensitive; do not reorder these definitions. */

s32 g_gosub_frame_parity;
s32 g_gosub_finished;
s32 g_gosub_cursor_row;
s32 g_gosub_row_count;
s32 g_gosub_visible_row_count;
/** @brief Screen-sequence cursor stored in a four-byte BSS slot. */
u8 g_gosub_screen_sequence_index;
u8 D_8016B8DD[3]; /* unreferenced; keeps the packed BSS layout */
s32 g_gosub_scroll_frames_remaining;
s32 g_gosub_block_shape;
s32 g_gosub_dialog_choice;
s32 g_gosub_block_level;
s32 g_gosub_allow_duplicate_selection;
/** @brief Encoded text currently displayed by the modal message dialog. */
u8* g_gosub_dialog_text;
s32 (*g_gosub_finish_handler)(void);
u8 g_gosub_selection_mode;
/** @brief Required selection count stored in a three-byte BSS slot. */
u8 g_gosub_required_selection_count;
u8 D_8016B8FE[2]; /* unreferenced; keeps the packed BSS layout */
/** @brief Row-detail flag stored in an eight-byte BSS slot. */
s32 g_gosub_show_row_details;
s32 D_8016B904; /* unreferenced; keeps the packed BSS layout */
s32 g_gosub_result_rows[16];
s32 g_gosub_dialog_accepting_input;
u8 g_gosub_selected_rows[4];
s32 g_gosub_window_height;
s32 (*g_gosub_select_handler)(void);
s32 g_gosub_window_width;
s32 g_gosub_suppress_dialog_sound;
u8 g_gosub_text_buffers[GOSUB_TEXT_BUFFER_COUNT][GOSUB_TEXT_BUFFER_SIZE];
/** @brief Current selection count stored in an eight-byte BSS slot. */
u8 g_gosub_selection_count;
u8 D_80170961[7]; /* unreferenced; keeps the packed BSS layout */
u8 g_gosub_screen_sequence[20];
s32 g_gosub_block_id;
/** @brief List row height stored in an eight-byte BSS slot. */
s32 g_gosub_row_height;
s32 D_80170984; /* unreferenced; keeps the packed BSS layout */
s32 g_gosub_scroll_y;
/** @brief Whether the next confirmed sort uses ascending order. */
s32 g_gosub_sort_ascending;
s32 g_gosub_scroll_target_y;
u8* g_gosub_title_text;
GosubElement g_gosub_elements[GOSUB_ELEMENT_COUNT];
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
    field_reset_input_repeat();
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
    s32 parity;
    s32 finished;

    field_text_reset_scratch();
    gosub_update_screen(render_context);
    field_text_upload_immediate_cache();
    parity = g_gosub_frame_parity;
    finished = g_gosub_finished;
    g_gosub_frame_parity = parity ^ 1;
    return finished;
}

/**
 * @brief Copy and enter a GOSUB_SCREEN_SEQUENCE_END-terminated screen sequence.
 * @param screen_sequence Sequence of screen ids stored as s32 values.
 * @see decomp.me (100%) https://decomp.me/scratch/weBhP
 */
void gosub_load_screen_sequence(s32* screen_sequence)
{
    s32 screen_count;
    s32 unused[2];

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
    for (screen_count = 0; screen_sequence[screen_count] != GOSUB_SCREEN_SEQUENCE_END; screen_count++)
    {
        g_gosub_screen_sequence[screen_count] = screen_sequence[screen_count];
    }
    g_gosub_screen_sequence[screen_count] = screen_sequence[screen_count];
    g_gosub_dialog_accepting_input = 0;
    gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
    gosub_upload_font_texture();
}

/**
 * @brief Initialize a gosub sub-screen and install its selection callbacks.
 * @param screen_id Screen id, 0..19; anything else returns without touching state.
 */
void gosub_enter_screen(s32 screen_id)
{
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    g_gosub_block_shape = 0;
    g_gosub_block_id = 0;
    g_gosub_block_level = 0;
    g_gosub_allow_duplicate_selection = 0;
    g_gosub_show_row_details = 0;

    switch (screen_id)
    {
    case GOSUB_SCREEN_PRIMARY_MATERIAL:
        gosub_close_elements();
        gosub_build_primary_material_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PRIMARY_MATERIAL));
        }
        break;

    case GOSUB_SCREEN_SECONDARY_MATERIAL:
        gosub_close_elements();
        gosub_build_secondary_material_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_SECONDARY_MATERIAL));
        }
        break;

    case GOSUB_SCREEN_WEAPON:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_WEAPON);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_equipment_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_WEAPONS));
        }
        break;

    case GOSUB_SCREEN_ARMOR:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_ARMOR);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_equipment_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_ARMOR));
        }
        break;

    case GOSUB_SCREEN_INSTRUMENT:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_INSTRUMENT);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_equipment_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_INSTRUMENTS));
        }
        break;

    case GOSUB_SCREEN_EQUIPMENT:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_ANY);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_equipment_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_EQUIPMENT));
        }
        break;

    case GOSUB_SCREEN_WEAPON_TYPE:
    case GOSUB_SCREEN_ARMOR_TYPE:
    case GOSUB_SCREEN_INSTRUMENT_TYPE:
        gosub_close_elements();
        gosub_build_equipment_type_list(screen_id - GOSUB_SCREEN_WEAPON_TYPE);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        break;

    case GOSUB_SCREEN_GOLEM_PARTS:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_GOLEM_PARTS);
        g_gosub_required_selection_count = 4;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_golem_part;
        g_gosub_finish_handler = gosub_publish_golem_parts;
        gosub_build_golem_parts_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_GOLEM_PARTS));
        }
        break;

    case GOSUB_SCREEN_BLOCK_COMPONENTS:
        gosub_close_elements();
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_ANY);
        g_gosub_show_row_details = 1;
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 16;
        g_gosub_window_width = 232;
        g_gosub_window_height = 100;
        g_gosub_block_shape = 0;
        g_gosub_block_id = 0;
        g_gosub_block_level = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = gosub_select_block_component;
        g_gosub_finish_handler = gosub_publish_block_components;
        g_gosub_dialog_handler = gosub_handle_make_block_dialog;
        gosub_build_block_components_elements();
        if (g_saved_game_ctx->logic_block_count >= LOGIC_BLOCK_CAPACITY)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_LOGIC_BLOCKS_FULL));
        }
        break;

    case GOSUB_SCREEN_LOGIC_BLOCKS:
        gosub_close_elements();
        gosub_build_logic_block_list();
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = gosub_select_logic_block;
        g_gosub_finish_handler = gosub_publish_selection;
        g_gosub_allow_duplicate_selection = 1;
        gosub_build_logic_block_list_elements();
        if (g_saved_game_ctx->logic_block_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_LOGIC_BLOCKS));
        }
        break;

    case GOSUB_SCREEN_COMPANION_TO_TAKE:
        gosub_close_elements();
        gosub_build_companion_list(GOSUB_COMPANIONS_ALL);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_companion_to_take;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_companion_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PETS_OR_GOLEMS));
        }
        break;

    case GOSUB_SCREEN_PET_TO_TAKE:
        gosub_close_elements();
        gosub_build_companion_list(GOSUB_COMPANIONS_PETS);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_companion_to_take;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_companion_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PETS));
        }
        break;

    case GOSUB_SCREEN_GOLEM:
        gosub_close_elements();
        gosub_build_companion_list(GOSUB_COMPANIONS_GOLEMS);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_companion_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_GOLEMS));
        }
        break;

    case GOSUB_SCREEN_COLOR_MATERIAL:
        gosub_close_elements();
        gosub_build_color_material_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_COLOR_MATERIAL));
        }
        break;

    case GOSUB_SCREEN_ELEMENTAL_COIN:
        gosub_close_elements();
        gosub_build_elemental_coin_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(0);
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_ELEMENTAL_COINS));
        }
        break;

    case GOSUB_SCREEN_COMPANION:
        gosub_close_elements();
        gosub_build_companion_list(GOSUB_COMPANIONS_ALL);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_companion_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PETS_OR_GOLEMS));
        }
        break;

    case GOSUB_SCREEN_PET:
        gosub_close_elements();
        gosub_build_companion_list(GOSUB_COMPANIONS_PETS);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_companion_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PETS));
        }
        break;

    case GOSUB_SCREEN_PRODUCE:
        gosub_close_elements();
        gosub_build_produce_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = gosub_select_row;
        g_gosub_finish_handler = gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_close_elements();
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_NO_PRODUCE));
        }
        break;
    }
}
