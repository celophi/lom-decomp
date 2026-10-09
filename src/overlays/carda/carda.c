#include "internal/carda.h"
#include "overlays/field/field_text.h"
#include "internal/carda_internal.h"
#include "main/audio/akao.h"

/**
 * @brief Card-slot label window positions: slot 0 and slot 1 X, and Y by layout.
 * @note JP moves the narrower slot 0 label right.
 */
#if defined(VERSION_JP)
#define CARDA_CARD_SLOT0_LABEL_X 44
#else
#define CARDA_CARD_SLOT0_LABEL_X 28
#endif
#define CARDA_CARD_SLOT1_LABEL_X 164
#define CARDA_CARD_LABEL_BROWSER_Y 30
#define CARDA_CARD_LABEL_POCKETSTATION_Y 58

/** @brief Title window of the browser layout. */
#define CARDA_TITLE_X 104
#define CARDA_TITLE_Y 10
#define CARDA_TITLE_WIDTH 112
#define CARDA_TITLE_HEIGHT 16

/** @brief Title window of the PocketStation layout. */
#define CARDA_POCKETSTATION_TITLE_X 32
#define CARDA_POCKETSTATION_TITLE_Y 34
#define CARDA_POCKETSTATION_TITLE_WIDTH 256

/** @brief Entry list columns of the rank marker and the "+" marker (right edge). */
#define CARDA_ENTRY_MARKER_X 214
#define CARDA_ENTRY_PLUS_RIGHT_X 268

/** @brief Bytes cleared when preparing the card header and icon area. */
#define CARDA_CARD_HEADER_BYTES 512

/** @brief Memory-card header icon flag: 16-color icon with two animation frames. */
#define CARDA_CARD_ICON_TWO_FRAMES 0x12

/** @brief Number of memory-card blocks a CARDA save file occupies. */
#define CARDA_SAVE_BLOCK_COUNT 2

/** @brief Number of save icons in the built-in save-icon table. */
#define CARDA_SAVE_ICON_COUNT 10

/** @brief One save icon: a 16-color CLUT followed by two 16x16 4-bit frames. */
typedef struct
{
    u16 clut[16];
    u8 frames[2][128];
} CardaSaveIcon;

/**
 * @brief Save icon @p icon in the built-in save-icon table, which holds an icon
 *        count followed by per-icon byte offsets from the table start.
 * @note g_carda_save_icon_offsets is the first offset, so the table starts one word before it.
 */
#define CARDA_SAVE_ICON(icon) ((CardaSaveIcon*)((u8*)&g_carda_save_icon_offsets - 4 + g_carda_save_icon_offsets[icon]))

/** @brief Left edge of the save window text, before the transition offset. */
#define CARDA_SAVE_TEXT_X (CARD_MENU_MESSAGE_WIDTH / 2)

/** @brief Item list stored in a memory-card save. */
typedef struct CardaSaveItemList
{
    s32 growth_delta; /**< Added to the restored record's growth counter. */
    u32 count;        /**< Number of valid entries in ids. */
    u8 ids[0x50];
} CardaSaveItemList;

/** @brief The parts of the memory-card save buffer (g_carda_save_blob) that CARDA reads. */
typedef struct CardaSaveData
{
    u8 unk0[0x300];
    CardaSaveItemList items;
    PetRecord record; /**< Pet restored into the saved game. */
} CardaSaveData;

/**
 * @brief The pet-transfer file in the save buffer.
 * @note g_carda_save_blob is a byte pointer because the buffer holds either a
 *       save file (SaveFile) or a pet-transfer file (CardaSaveData).
 */
#define CARDA_SAVE_DATA ((CardaSaveData*)g_carda_save_blob)

static void* carda_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_build_ui_elements(void);
static void carda_update_menu(FieldRenderHalf* render);
static s32 carda_update_card_sequence(void);
static s32 carda_handle_input(void);
static void carda_switch_card(void);
static void carda_close_all_elements(void);
static void carda_update_elements(FieldRenderHalf* render);
static void* carda_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_title(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_update_and_draw_elements(FieldRenderHalf* render);
static void* carda_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_save_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_save_complete(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_format_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
static void carda_restore_active_record(void);
static void* carda_draw_slot_prompt(void* prim, u_long* ot, s32 x, s32 y);
static void* carda_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_open_item_list(void);
static void* carda_draw_item_list_header(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_apply_save_items(void);

/**
 * @brief Reset the overlay state, clear the icon and glyph VRAM and build the windows.
 * @param work Work buffer from FIELD; the save file is built and read in it.
 * @param mode Card screen mode (CARDA_MODE_SAVE and so on), stored in g_carda_mode.
 */
void carda_init(void* work, s32 mode)
{
    RECT rect;

    g_carda_mode = mode;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;
    carda_reset_entry_ranks();
    carda_init_card_events();
    g_carda_icon_phase = 0;
    field_set_default_fade_target();

    setRECT(&rect, OVERLAY_INIT_CLEAR_VRAM_X, OVERLAY_INIT_CLEAR_VRAM_Y, OVERLAY_INIT_CLEAR_VRAM_W, OVERLAY_INIT_CLEAR_VRAM_H);
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();

    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = CARD_MENU_SELECTION_NONE;
    g_carda_io_busy = 0;
    g_carda_frame_parity = 0;
    g_carda_exit_requested = 0;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
    }
    g_card_step = NULL;
    g_carda_choice_toggle = g_card_slot;
    field_reset_input_repeat();
    carda_build_ui_elements();

    g_carda_save_blob = (u8*)(((uintptr_t)work + 3) & ~3);
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param render FIELD render buffer being built this frame.
 * @return 1 when the overlay has finished, otherwise 0.
 */
s32 carda_update_frame(FieldRenderHalf* render)
{
    if (g_carda_exit_requested != 0)
    {
        shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    begin_glyph_cache_frame();
    carda_update_menu(render);
    evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_carda_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll and selection state and open the windows of the current mode.
 *
 * The PocketStation modes get the transfer window, both card-slot labels and
 * the title; the save and load modes get the save-file browser: entry list,
 * title, both card-slot labels and the details window.
 */
static void carda_build_ui_elements(void)
{
    s32 unused[2];
    CardMenuElement* element;

    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = CARD_MENU_SELECTION_NONE;
    g_carda_items = g_saved_game_ctx->items;
    carda_clear_elements();
    g_carda_format_declined = 0;

    g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        element = carda_alloc_element();
        element->draw = carda_draw_save_flow;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARDA_TRANSFER_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARDA_TRANSFER_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_POCKETSTATION_Y;
        element->size.bits.width_high = CARD_MENU_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_POCKETSTATION_Y;
        element->size.bits.width_high = CARD_MENU_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_title;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_POCKETSTATION_TITLE_X;
        element->attr.bits.y = CARDA_POCKETSTATION_TITLE_Y;
        element->size.bits.width_high = CARDA_POCKETSTATION_TITLE_WIDTH >> 8;
        element->size.bits.height = CARDA_TITLE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARDA_POCKETSTATION_TITLE_WIDTH);
    }
    else
    {
        element = carda_alloc_element();
        element->draw = carda_draw_entry_list;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_LIST_X;
        element->attr.bits.y = CARD_MENU_LIST_Y;
        element->size.bits.width_high = CARDA_LIST_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_LIST_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARDA_LIST_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_title;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_TITLE_X;
        element->attr.bits.y = CARDA_TITLE_Y;
        element->size.bits.width_high = CARDA_TITLE_WIDTH >> 8;
        element->size.bits.height = CARDA_TITLE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARDA_TITLE_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_BROWSER_Y;
        element->size.bits.width_high = CARD_MENU_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_BROWSER_Y;
        element->size.bits.width_high = CARD_MENU_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_selected_entry_details;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_DETAILS_X;
        element->attr.bits.y = CARD_MENU_DETAILS_Y;
        element->size.bits.width_high = CARD_MENU_DETAILS_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_DETAILS_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DETAILS_WIDTH);
    }
    g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

/**
 * @brief Run one frame of overlay logic: update the windows, advance the card
 *        sequence once the main window is open, handle input and step the scroll.
 * @param render FIELD render buffer being built this frame.
 */
static void carda_update_menu(FieldRenderHalf* render)
{
    carda_update_elements(render);
    g_carda_icon_phase += 2;
    if (g_carda_element1_state.attr.bits.state == CARD_MENU_ELEMENT_OPEN && g_carda_element1_state.attr.bits.transition_step == 0)
    {
        carda_update_card_sequence();
    }
    if ((u16)g_pad_input == PAD_ALL_BUTTONS)
    {
        g_pad_input = 0;
    }
    carda_handle_input();
    if (g_carda_scroll_frames != 0)
    {
        g_carda_scroll_y += (g_carda_scroll_target_y - g_carda_scroll_y) / g_carda_scroll_frames;
        g_carda_scroll_frames--;
    }
    else
    {
        g_carda_scroll_y = g_carda_scroll_target_y;
    }
}

/**
 * @brief Drive the card sequence one frame and act on its result.
 *
 * Starts the initial scan when no sequence is running and no status message
 * is up, runs the sequence until it stops asking to run again, reopens the
 * format prompt when the player confirms after declining it, and maps the
 * sequence result onto the next step table or the unformatted-card prompt.
 *
 * @return Unspecified; callers use the updated card state.
 */
static s32 carda_update_card_sequence(void)
{
    s32 result;
    CardMenuElement* prompt;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        if (g_card_step == NULL)
        {
            switch (g_card_entry_state)
            {
            case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
            case CARDA_ENTRY_STATE_FORMAT_FAILED:
            case CARDA_ENTRY_STATE_SAVE_CORRUPT:
            case CARDA_ENTRY_STATE_NO_POCKETSTATION:
            case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
            case CARDA_ENTRY_STATE_UPLOAD_FAILED:
            case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
            case CARDA_ENTRY_STATE_SELECT_SLOT:
            case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
            case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
            case CARD_MENU_ENTRY_STATE_UNFORMATTED:
            case CARD_MENU_ENTRY_STATE_CARD_FULL:
            case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
            case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
            case CARD_MENU_ENTRY_STATE_NO_CARD:
            case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
                return;
            default:
                g_card_step = g_carda_steps_initial_scan;
                break;
            }
        }
    }
    else if (g_card_entry_state >= CARDA_ENTRY_COUNT_INPUT_LIMIT && g_card_step == NULL && g_card_entry_state != CARDA_ENTRY_STATE_SELECT_SLOT)
    {
        g_card_step = g_carda_steps_initial_scan;
    }

    do
    {
        result = carda_advance_card_sequence();
    } while (result == CARD_MENU_SEQUENCE_RUN_AGAIN);

    if (g_carda_format_declined != 0 && (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK))
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
        g_carda_format_declined = 0;

        prompt = g_carda_element_pool;
        prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        prompt->attr.bits.transition_step = 1;
        prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
        prompt->attr.bits.y = CARDA_TRANSFER_Y;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
        prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        prompt->size.bits.height = CARDA_TRANSFER_HEIGHT;
        carda_enable_choice_toggle();
        prompt->draw = carda_draw_format_prompt;
        restart_card_sequence();
        return;
    }

    switch (result)
    {
    case CARD_MENU_SEQUENCE_NONE:
        break;
    case CARD_MENU_SEQUENCE_FINISHED:
        g_card_step = g_carda_steps_card_reset;
        break;
    case CARD_MENU_SEQUENCE_NO_CARD:
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_card_step = NULL;
        }
        else
        {
            g_card_step = g_carda_steps_refresh_entries;
        }
        g_carda_format_declined = 0;
        break;
    case CARD_MENU_SEQUENCE_UNFORMATTED:
        if (g_carda_mode == CARDA_MODE_LOAD || g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
            if (g_carda_mode == CARDA_MODE_LOAD)
            {
                g_card_step = g_carda_steps_initial_scan;
            }
        }
        else
        {
            g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
            g_carda_format_declined = 0;

            prompt = g_carda_element_pool;
            prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
            prompt->attr.bits.transition_step = 1;
            prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
            prompt->attr.bits.y = CARDA_TRANSFER_Y;
            CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
            prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
            prompt->size.bits.height = CARDA_TRANSFER_HEIGHT;
            carda_enable_choice_toggle();
            prompt->draw = carda_draw_format_prompt;
            restart_card_sequence();
        }
        break;
    }
}

/**
 * @brief Handle browser input: cancel, card switch, entry navigation and confirm.
 * @return Unspecified; callers use the updated menu state.
 */
static s32 carda_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    CardMenuElement* prompt;

    if (g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_FREE)
    {
        g_carda_exit_requested = 1;
        return;
    }
    if (g_carda_exit_requested != 0)
    {
        return;
    }
    if (g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state >= CARD_MENU_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state != CARD_MENU_ELEMENT_FREE)
    {
        return;
    }
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        return;
    }
    entry_count = g_card_entry_state;
    if (entry_count == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
    {
        return;
    }
    if (g_carda_entry_scan_active != 0)
    {
        return;
    }
    if (g_carda_io_busy != 0)
    {
        return;
    }
    if (*g_card_step >= CARD_MENU_STEP_SCAN_ENTRIES && *g_card_step <= CARD_MENU_STEP_SCAN_DONE)
    {
        return;
    }

    input = g_pad_input;
    if (input & PAD_BTN_CIRCLE)
    {
        g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        carda_close_all_elements();
        return;
    }
    if (input & CARD_MENU_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        carda_switch_card();
        return;
    }
    if (entry_count >= CARDA_ENTRY_COUNT_INPUT_LIMIT)
    {
        return;
    }

    move_count = 1;
    if (input & PAD_BTN_R1)
    {
        g_pad_input = PAD_BTN_DOWN;
        move_count = 1;
    }
    if (g_pad_input & PAD_BTN_L1)
    {
        g_pad_input = PAD_BTN_UP;
        move_count = 1;
    }

    for (; move_count != 0; move_count--)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_carda_selected_row--;
            if (g_carda_selected_row < 0)
            {
                g_carda_selected_row = g_card_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_carda_selected_row++;
            if (g_carda_selected_row >= g_card_entry_state)
            {
                g_carda_selected_row = 0;
            }
        }
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        carda_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        carda_scroll_to_selection();
        return;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        if (g_carda_mode == CARDA_MODE_LOAD)
        {
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                if (g_save_compatibility_tag == SAVE_TAG_ANY || g_carda_selected_file.saved_game.compatibility_tag == g_save_compatibility_tag)
                {
                    prompt = carda_alloc_element();
                    prompt->attr.bits.transition_step = 1;
                    prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                    prompt->attr.bits.y = CARDA_MESSAGE_Y;
                    prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                    prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                    CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
                    carda_build_save_file();
                    carda_enable_choice_toggle();
                    prompt->draw = carda_draw_load_prompt;
                    restart_card_sequence();
                    field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                    return;
                }
            }
        }
        else
        {
            if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
            {
                prompt = carda_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
                carda_build_save_file();
                carda_enable_choice_toggle();
                prompt->draw = carda_draw_save_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                return;
            }
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                prompt = carda_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
                carda_build_save_file();
                carda_enable_choice_toggle();
                prompt->draw = carda_draw_overwrite_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    }
}

/**
 * @brief Switch to the other card slot and restart its directory scan.
 */
static void carda_switch_card(void)
{
    g_carda_format_declined = 0;
    g_card_step = NULL;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = CARD_MENU_SELECTION_NONE;
    g_card_slot ^= 1;
    carda_reset_entry_ranks();
    clear_hardware_card_events();
    clear_software_card_events();
}

/**
 * @brief Restore the field fade target and start closing every open window.
 */
static inline void carda_close_all_elements(void)
{
    CardMenuElement* element;
    s32 i;

    field_restore_fade_target();
    element = g_carda_element_pool;
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
 * @brief Retarget the list scroll so the selected row stays in the window.
 */
inline void carda_scroll_to_selection(void)
{
    s32 row_y;
    s32 relative_y;

    row_y = g_carda_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
    relative_y = row_y - g_carda_scroll_y;
    if (relative_y > CARD_MENU_LIST_HEIGHT - CARD_MENU_ENTRY_ROW_HEIGHT)
    {
        g_carda_scroll_target_y = row_y - CARD_MENU_LIST_LAST_ROW_Y;
        g_carda_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
    if (relative_y < 0)
    {
        g_carda_scroll_target_y = g_carda_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
        g_carda_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
}

/**
 * @brief Run the window update and draw pass.
 * @param render FIELD render buffer being built this frame.
 */
static void carda_update_elements(FieldRenderHalf* render)
{
    carda_update_and_draw_elements(render);
}

/**
 * @brief Draw the save-file browser: the message for a status entry state, or
 *        one row per directory entry and the selected-row highlight.
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    /* A prompt or dialog covers the status messages, except the blank one. */
    if (g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state != CARD_MENU_ELEMENT_FREE)
    {
        if (g_card_entry_state >= CARD_MENU_ENTRY_STATE_NO_GAME_DATA)
        {
            if (g_card_entry_state < CARD_MENU_ENTRY_STATE_BLANK)
            {
                return prim;
            }
            if (g_card_entry_state == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
            {
                return prim;
            }
        }
    }

    switch (g_card_entry_state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_no_lom_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_unformatted, CARD_MENU_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table = &g_carda_text_checking_card;

        prim =
            field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table;

        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table;

        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DOWNLOAD_RING_RING_LAND), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NEEDS_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_DETAILS_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_no_card, CARD_MENU_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_LIST_WIDTH / 2,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    default:
    {
        s32 row_y;
        s32 i;

        if (g_carda_entry_scan_active != 0)
        {
            s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
            u16* text_table = &g_carda_text_checking_card;

            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
            break;
        }

        i = 0;
        if (i < g_card_entry_state)
        {
            s32 list_x;
            u16 marker_offset;
            Vec2s value_pos;
            u16* text_table;
            s32 marker_x;
            s32 marker_text_x;
            s32 color;
            s32 label_x;

            text_table = &g_carda_text_checking_card;
            list_x = -x_offset;
            for (; i < g_card_entry_state; i++)
            {
                color = FIELD_TEXT_COLOR_NORMAL;
                marker_x = list_x + CARDA_ENTRY_MARKER_X;
                label_x = CARD_MENU_ENTRY_LABEL_X - x_offset;
                row_y = ((i * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y + 1;
                if (row_y > -CARD_MENU_ENTRY_ROW_HEIGHT && row_y < CARD_MENU_LIST_HEIGHT)
                {
                    if (g_carda_entry_ranks[i] >= 0)
                    {
                        value_pos.x = list_x + CARD_MENU_ENTRY_VALUE_X;
                        value_pos.y = row_y;
                        prim = field_draw_number(ot, prim, g_card_entry_suffix_values[i], color, &value_pos, FIELD_TEXT_ALIGN_LEFT);
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_number_label), color,
                                               list_x + CARD_MENU_ENTRY_NUMBER_LABEL_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        value_pos.y = row_y;
                        value_pos.x = marker_x;
                        if ((g_carda_rank_count - 1) == g_carda_entry_ranks[i])
                        {
                            marker_offset = text_table[CARD_MENU_TEXT_NEWEST];
                            marker_text_x = (s16)marker_x;
                            prim =
                                field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_text_x, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        else if (g_carda_entry_ranks[i] < 2)
                        {
                            marker_offset = text_table[CARD_MENU_TEXT_OLDEST];
                            marker_text_x = (s16)marker_x;
                            prim =
                                field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_text_x, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        if (*skip_hex_digits(&g_card_entries[g_card_slot][i].name[CARD_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_plus_marker), color,
                                                   CARDA_ENTRY_PLUS_RIGHT_X - x_offset, row_y, FIELD_TEXT_ALIGN_RIGHT);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][i].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim =
                            field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_mana_label), color, label_x, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][i].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_ring_ring_land_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][i].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_new_save_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_card_full_entry_name, g_card_entries[g_card_slot][i].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_card_full_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_carda_text_other_game_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                }
            }
        }

        row_y = ((g_carda_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y;
        if (g_carda_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = CARD_MENU_HIGHLIGHT_COLOR;
            setlen(tile, 3);
            setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
            tile->w = CARDA_LIST_WIDTH;
            setXY0(tile, 0, row_y);
            tile->h = CARD_MENU_ENTRY_ROW_HEIGHT;
            addPrim(ot, tile);
            prim = tile + 1;
        }
        break;
    }
    }
    return prim;
}

/**
 * @brief Draw the title of the current mode: "Save", "Load" or Ring Ring Land.
 * @param ot Ordering table the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_title(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_ring_ring_land_title, CARD_MENU_TEXT_RING_RING_LAND_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_POCKETSTATION_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else if (g_carda_mode == CARDA_MODE_LOAD)
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_load_title, CARD_MENU_TEXT_LOAD_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_save_title, CARD_MENU_TEXT_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }

    return prim;
}

/**
 * @brief Draw the slot 1 card label, dimmed while the other slot is selected.
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    TILE* tile;

    if (g_card_slot != 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = CARD_MENU_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, CARD_MENU_CARD_LABEL_WIDTH, CARD_MENU_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_slot0_label, CARD_MENU_TEXT_CARD_SLOT0_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARD_MENU_CARD_LABEL_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the slot 2 card label, dimmed while the other slot is selected.
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    TILE* tile;

    if (g_card_slot == 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = CARD_MENU_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, CARD_MENU_CARD_LABEL_WIDTH, CARD_MENU_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_slot1_label, CARD_MENU_TEXT_CARD_SLOT1_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARD_MENU_CARD_LABEL_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the details of the selected entry.
 *
 * Shows the new-save or full-card notice, the party icons, play time, hero
 * name and location of a Legend of Mana save (or the wrong-version message),
 * or the two-line card title of any other game's save.
 *
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note The JP layout places the time and name columns farther left.
 */
static void* carda_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    Vec2s pos;
    u8 name[256];
    s32 party_icon[FIELD_PARTY_SIZE];
    DVECTOR unused;

    result = prim;
    if (g_carda_selection_status == CARD_MENU_SELECTION_NONE)
    {
        return result;
    }
    if (g_carda_entry_scan_active != 0)
    {
        return result;
    }
    if (g_carda_selection_status == CARD_MENU_SELECTION_EMPTY_CARD)
    {
        return result;
    }
    if (g_card_entry_state == CARD_MENU_ENTRY_STATE_CARD_FULL)
    {
        return result;
    }
    if (g_card_entry_state >= CARD_MENU_ENTRY_COUNT_LIMIT)
    {
        return result;
    }
    if (g_carda_selection_status == CARD_MENU_SELECTION_NEW_SAVE)
    {
        s32 x = -x_offset;
        u16* text_table;

        result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                 FIELD_TEXT_ALIGN_LEFT);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE);
        return field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_USES_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_DETAILS_LINE_HEIGHT - y_offset,
                               FIELD_TEXT_ALIGN_LEFT);
    }
    else if (g_carda_selection_status == CARDA_SELECTION_CARD_FULL)
    {
        return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_needs_two_blocks, CARD_MENU_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, -x_offset,
                               -y_offset, FIELD_TEXT_ALIGN_LEFT);
    }
    else
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            if (g_save_compatibility_tag == SAVE_TAG_ANY || g_carda_selected_file.saved_game.compatibility_tag == g_save_compatibility_tag ||
                g_carda_selected_file.saved_game.compatibility_tag == SAVE_TAG_ANY)
            {
                SavedGameLayout* save = &g_carda_selected_file.saved_game;
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
                g_carda_icon_palette = save->icon_palette;

                present_count = 0;
                for (i = 0; i < FIELD_PARTY_SIZE; i++)
                {
                    if (party_icon[i] != SAVE_NO_ICON)
                    {
                        present_count += 1;
                    }
                }

                switch (present_count)
                {
                case 2:
                    step = 32;
                    half_step = 16;
                    g_carda_icon_phase %= 32;
                    break;
                case 3:
                    step = 16;
                    half_step = 32;
                    g_carda_icon_phase %= 96;
                    break;
                default:
                    step = 16;
                    half_step = 32;
                    g_carda_icon_phase = 31;
                    break;
                }

                i = 0;
                j = i;
                for (; j < FIELD_PARTY_SIZE; j++)
                {
                    base_y = i * half_step;
                    base_x = base_y + half_step;
                    if (party_icon[j] != SAVE_NO_ICON)
                    {
                        s32 adjust = step;
                        s32 rem;
                        s32 hi;

                        if (g_carda_icon_phase >= base_y && g_carda_icon_phase < base_x)
                        {
                            adjust += g_carda_icon_phase - base_y;
                        }
                        else
                        {
                            rem = base_x % (half_step * present_count);
                            if (g_carda_icon_phase >= rem)
                            {
                                hi = rem + half_step;
                                if (g_carda_icon_phase < hi)
                                {
                                    adjust += hi - g_carda_icon_phase;
                                }
                            }
                        }
                        result = carda_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, party_icon[j], i, j);
                        total += adjust;
                        i += 1;
                    }
                }

                {
                    SavedGameLayout* shown_save = &g_carda_selected_file.saved_game;
                    s32 x = -x_offset;
                    s32 y = -y_offset;

                    base_y = shown_save->play_time;

                    pos.x = x + CARD_MENU_DETAILS_HOURS_RIGHT_X;
                    pos.y = y;
                    hours = base_y / SAVED_PLAY_TIME_TICKS_PER_HOUR;
                    result = field_draw_number(ot, result, hours, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, FIELD_UI_TEXT_TIME_SEPARATOR),
                                             FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TIME_SEPARATOR_X, y, FIELD_TEXT_ALIGN_LEFT);
                    base_y = (base_y / SAVED_PLAY_TIME_TICKS_PER_MINUTE) - (hours * 60);
                    if (base_y < 10)
                    {
                        pos.x = x + CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X;
                        pos.y = y;
                        result = field_draw_number(ot, result, 0, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    }
                    pos.x = x + CARD_MENU_DETAILS_MINUTES_RIGHT_X;
                    pos.y = y;
                    result = field_draw_number(ot, result, base_y, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, shown_save->summary_name, FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TEXT_X,
                                             y + CARD_MENU_DETAILS_LINE_HEIGHT, FIELD_TEXT_ALIGN_LEFT);
                    result = field_draw_text(result, ot, CARD_MENU_TEXT(g_carda_location_names, shown_save->track.bits.music_track), FIELD_TEXT_COLOR_NORMAL,
                                             x + CARD_MENU_DETAILS_TEXT_X, y + CARD_MENU_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                }
            }
            else
            {
                result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_carda_text_wrong_version, CARD_MENU_TEXT_WRONG_VERSION), FIELD_TEXT_COLOR_NORMAL, -x_offset,
                                         -y_offset, FIELD_TEXT_ALIGN_LEFT);
            }
        }
        else
        {
            s32 j;
            SaveFileHeader* header;

            terminate_multibyte_text(g_carda_selected_file.header.title);
            header = &g_carda_selected_file.header;
            if (header->title[1][0] == 0 || header->title[1][0] >= SJIS_LEAD_MIN)
            {
                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = *(header->title[0] + j);
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);

                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = g_carda_selected_file.header.title[1][j];
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset + CARD_MENU_DETAILS_LINE_HEIGHT, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);
            }
        }
    }
    return result;
}

#include "../../common/save_file/terminate_multibyte_text.inc.c"

/**
 * @brief Draw FIELD's "Can't hold any more." notice, centred in a 256-pixel window.
 * @param ot Ordering table the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Nothing in CARDA uses it.
 */
void* carda_draw_cant_hold_more(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    return field_draw_text(prim, ot, FIELD_UI_TEXT_AT(g_field_ui_text_cant_hold_more, FIELD_UI_TEXT_CANT_HOLD_MORE), FIELD_TEXT_COLOR_DIM,
                           CARDA_ITEM_LIST_WIDTH / 2 - x_offset, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Free every element of the pool and select the sub-overlay frame style.
 */
inline void carda_clear_elements(void)
{
    CardMenuElement* p;
    s32 i;

    g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
    p = g_carda_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++)
    {
        p->attr.bits.state = CARD_MENU_ELEMENT_FREE;
        p++;
    }
}

/**
 * @brief Claim the first free pool element and start its opening transition.
 * @return The claimed element, or the pool base element when none is free.
 */
inline CardMenuElement* carda_alloc_element(void)
{
    CardMenuElement* p;
    s32 i;

    p = g_carda_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == CARD_MENU_ELEMENT_FREE)
        {
            p->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
            return p;
        }
    }
    return g_carda_element_pool;
}

/**
 * @brief Advance and draw the pool elements for one frame.
 *
 * Emits the list scroll arrows, then links a draw-environment packet for each
 * live element into the ordering table and animates it by state: an opening
 * window grows, an open window holds, a closing window shrinks, and a closed
 * window counts down to free.
 *
 * @param render FIELD render buffer; its primitive cursor is read on entry and written back on exit.
 */
static void carda_update_and_draw_elements(FieldRenderHalf* render)
{
    void* prim;
    u_long* ot;
    CardMenuElement* element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    prim = render->primitive_cursor;
    ot = render->ordering_table;

    if (g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].draw == carda_draw_entry_list)
    {
        if ((g_card_entry_state < CARD_MENU_ENTRY_COUNT_LIMIT) && (g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN))
        {
            if ((g_card_entry_state * CARD_MENU_ENTRY_ROW_HEIGHT) > (g_carda_scroll_y + CARD_MENU_LIST_HEIGHT))
            {
                prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
            }
            if (g_carda_scroll_y != 0)
            {
                prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
            }
        }
    }
    else if ((g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].draw == carda_draw_item_list) &&
             (g_carda_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN))
    {
        if (((g_carda_received_item_count * CARD_MENU_LINE_HEIGHT) - g_carda_scroll_y) > CARDA_ITEM_LIST_VISIBLE_ROWS * CARD_MENU_LINE_HEIGHT)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_ITEM_SCROLL_ARROW_X, CARDA_ITEM_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
        }
        if (g_carda_scroll_y != 0)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_ITEM_SCROLL_ARROW_X, CARDA_ITEM_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
        }
    }

    if (render->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }

    element = g_carda_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != CARD_MENU_ELEMENT_FREE)
        {
            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.bits.state)
            {
            case CARD_MENU_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARD_MENU_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 render->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
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

                    prim = field_draw_menu_frame(prim, ot, element->attr.bits.x, element->attr.bits.y, CARD_MENU_ELEMENT_WIDTH(element, width_low),
                                                 element->size.bits.height, render->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
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

                    scaled_width = (width * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 render->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = CARD_MENU_ELEMENT_CLOSED_FRAMES;
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

    render->primitive_cursor = prim;
}

/**
 * @brief Free the modal window's pool element.
 */
inline void carda_deactivate_primary_element(void)
{
    g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"

/**
 * @brief Build the save file for the live saved game in g_carda_save_blob: card
 *        header with title and icon, updated file-select summary, a copy of
 *        the saved game, and its checksum.
 * @note In overlay modes 2 and 3 (g_carda_mode) it only calls carda_store_active_record.
 */
void carda_build_save_file(void)
{
    SaveFile* blob;
    u8* cursor;
    u8* src;
    u8* title_text;
    s8* text;
    s32 i;
    s32 count;
    s32 icon;
    s32 time;
    s32 hours;
    u32 party_icon;
    s32 seed;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        carda_store_active_record();
        return;
    }

    /* Header: magic, icon format and size, then clear the rest of the standard card header. */
    blob = (SaveFile*)g_carda_save_blob;
    cursor = (u8*)blob + CARDA_CARD_HEADER_BYTES - 5;
    blob->header.magic[0] = 'S';
    blob->header.magic[1] = 'C';
    blob->header.icon_flags = CARDA_CARD_ICON_TWO_FRAMES;
    blob->header.block_count = CARDA_SAVE_BLOCK_COUNT;
    for (i = CARDA_CARD_HEADER_BYTES - 5; i >= 0; i--)
    {
        cursor[4] = 0;
        cursor--;
    }

    icon = g_saved_game_ctx->control.fields.placed_land_count * 9 / 26;
    i = 0;
    if (icon >= CARDA_SAVE_ICON_COUNT)
    {
        icon = CARDA_SAVE_ICON_COUNT - 1;
    }
    src = (u8*)CARDA_SAVE_ICON(icon);
    while (i < 32)
    {
        blob->header.clut[i] = *src;
        i++;
        src++;
    }
    count = 0;
    while (count < 2)
    {
        i = 0;
        while (i < 128)
        {
            blob->header.icon_frames[count][i] = *src;
            i++;
            src++;
        }
        count++;
    }

    /* Title: template text, optional marker, save number, play time and the player's name. */
    strcpy(blob->header.title[0], g_carda_save_title_template);
    if (g_saved_game_ctx->options.bits.flag_2)
    {
        blob->header.title[0][8] = 0x81;
        blob->header.title[0][9] = 0xF4;
    }
    time = g_saved_game_ctx->play_time + VSync(-1) - g_playtime_vsync_origin;
    g_saved_game_ctx->play_time = time;
    g_playtime_vsync_origin = VSync(-1);
    text = format_decimal((s8*)&blob->header.title[0][0x12], g_card_entry_suffix_values[g_carda_selected_row]);
    strcpy(text, "\x81\x7c"); /* Shift-JIS full-width minus */
    hours = time / SAVED_PLAY_TIME_TICKS_PER_HOUR;
    text = format_decimal(text + 2, hours);
    strcpy(text, "\x81\x46"); /* Shift-JIS full-width colon */
    time = time / SAVED_PLAY_TIME_TICKS_PER_MINUTE - hours * 60;
    text += 2;
    if (time < 10)
    {
        text = format_decimal(text, 0);
    }
    expand_text_glyph_codes((u8*)format_decimal(text, time), g_saved_game_ctx->characters[FIELD_PARTY_HERO].name);

    /* File-select summary in the live saved game. */
    g_saved_game_ctx->spawn.bits.party_icon_0 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].info.bits.type;
    if (g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0] != 0)
    {
        party_icon = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bits.type;
        if (party_icon < 2)
        {
            g_saved_game_ctx->track.bits.party_icon_1 = party_icon;
        }
        else
        {
            g_saved_game_ctx->track.bits.party_icon_1 = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bytes[1] + 2;
        }
    }
    else
    {
        g_saved_game_ctx->track.bits.party_icon_1 = SAVE_NO_ICON;
    }
    if (g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].name[0] != 0)
    {
        if (g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bits.type == FIELD_CHARACTER_GOLEM)
        {
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + SAVE_ICON_GOLEM_BASE;
            g_saved_game_ctx->icon_palette = g_saved_game_ctx->golem_records[g_saved_game_ctx->joined_golem].palette;
        }
        else
        {
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + SAVE_ICON_PET_BASE;
        }
    }
    else
    {
        g_saved_game_ctx->track.bits.party_icon_2 = SAVE_NO_ICON;
    }
    for (i = 0; i < 21; i++)
    {
        g_saved_game_ctx->summary_name[i] = g_saved_game_ctx->characters[FIELD_PARTY_HERO].name[i];
    }
    g_saved_game_ctx->unk15 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].progress.level;
    g_saved_game_ctx->unk16 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].equipment[FIELD_WEAPON_SLOT].derived.bytes[0];
    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (g_saved_game_ctx->summary_records[i].in_use != 0)
        {
            count++;
        }
    }
    g_saved_game_ctx->summary_slot_count = count;

    /* The copy runs 8 bytes past saved_game; the checksum and marker overwrite them. */
    bcopy((u8*)g_saved_game_ctx, (u8*)&blob->saved_game, SAVED_GAME_DATA_SIZE);
    blob->saved_game.spawn.bits.id = FIELD_SPAWN_LOAD_GAME;
    seed = rand();
    blob->saved_game.identity.ids.save_id = seed | (rand() << 15);
    blob->checksum = compute_save_checksum(blob);
    blob->magic = SAVE_FILE_MAGIC;

    /* Replace the start of the title with its final text. */
    src = blob->header.title[0];
    title_text = g_carda_bad_title_template;
    count = 0;
    while (count < 18)
    {
        count++;
        *src++ = *title_text++;
    }
}

#include "../../common/save_file/skip_hex_digits.inc.c"

/**
 * @brief Test SAVED_OPTION_FLAG_2 of the live saved game in overlay mode 0.
 * @return 1 when g_carda_mode is 0 and the flag is set, otherwise 0.
 */
s32 carda_test_option_flag_2(void)
{
    if (g_carda_mode == CARDA_MODE_SAVE && g_saved_game_ctx->options.bits.flag_2)
    {
        return 1;
    }
    return 0;
}

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"

/**
 * @brief Draw the load confirmation prompt and handle its input: a card
 *        change closes the prompt, cancel or "no" returns to the card reset
 *        steps, and "yes" starts loading the selected save.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* prompt;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_load_prompt, CARD_MENU_TEXT_LOAD_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                                                      -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        carda_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_card_step = g_carda_steps_initial_scan;
        }
        else
        {
            g_card_step = NULL;
        }
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            carda_deactivate_primary_element();
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                carda_deactivate_primary_element();
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_active = 1;
                g_card_step = g_carda_steps_load_selected_save;
                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_load_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading" message and progress bar, then commit the save
 *        file to the live saved game once the read has finished and validates.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    SaveFile* blob;
    CardMenuElement* element;
    CardMenuElement* cursor;
    void* result;
    u16* text_table;
    s32 x;
    s32 i;
    s32 valid;
    s32 checksum;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_carda_text_loading, CARD_MENU_TEXT_LOADING);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_progress_active == 0)
    {
        blob = (SaveFile*)g_carda_save_blob;
        element = g_carda_element_pool;
        element->attr.bits.state = CARD_MENU_ELEMENT_FREE;
        checksum = compute_save_checksum(blob);
        valid = 0;
        if (blob->checksum == checksum)
        {
            valid = blob->magic == SAVE_FILE_MAGIC;
        }
        if (valid == 0)
        {
            carda_open_status_dialog(CARDA_DIALOG_SAVE_CORRUPT);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        bcopy((u8*)&blob->saved_game, (u8*)g_saved_game_ctx, SAVED_GAME_DATA_SIZE);
        g_playtime_vsync_origin = VSync(-1);
        carda_close_all_elements();
        field_set_fade_target(0, 0, 0, 8);
    }

    return result;
}

/**
 * @brief Emit the time-based memory-card progress bar as a gouraud-shaded quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged while
 *         g_carda_progress_bar_active is 0.
 * @note The bar is 288 pixels wide after 256 frames since g_carda_progress_start_tick; in
 *       overlay modes 2 and 3 it runs 16 times faster for entry state 0xF4
 *       and three times slower otherwise.
 */
void* carda_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    POLY_G4* bar;
    s32 elapsed;
    s32 width;

    if (g_carda_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_carda_progress_start_tick;
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            if (g_card_entry_state == CARDA_ENTRY_STATE_UPLOADING)
            {
                elapsed *= 16;
            }
            else
            {
                elapsed /= 3;
            }
        }
        if (elapsed > CARD_MENU_PROGRESS_FULL_TICKS)
        {
            elapsed = CARD_MENU_PROGRESS_FULL_TICKS;
        }
        SET_BGR0_PACKED(quad, CARD_MENU_PROGRESS_TOP_LEFT_COLOR);
        SET_POLY_G4_BGR1_PACKED(quad, CARD_MENU_PROGRESS_TOP_RIGHT_COLOR);
        SET_POLY_G4_BGR3_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR);
        setPolyG4(quad);
        SET_POLY_G4_BGR2_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR);
        /* TODO: recover a structured form for the horizontal bounds. */
        do
        {
            do
            {
                bar = quad;
                quad->x2 = 0;
            } while (0);
            width = elapsed * CARD_MENU_MESSAGE_WIDTH;
            quad->x0 = 0;
            if (width < 0)
            {
                bar = quad;
                width += 255;
            }
            quad->x3 = width >> 8;
        } while (0);
        quad->x1 = width >> 8;
        quad = bar + 1;
        bar->y1 = 0;
        bar->y0 = 0;
        bar->y3 = CARDA_TRANSFER_HEIGHT;
        bar->y2 = CARDA_TRANSFER_HEIGHT;
        addPrim(ot, bar);
    }
    return quad;
}

/**
 * @brief Draw the "Save?" prompt for a new save file and handle its input;
 *        "yes" starts writing the save file.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_save_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* prompt;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_save_prompt, CARD_MENU_TEXT_SAVE_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                                                      -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        carda_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            carda_deactivate_primary_element();
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                carda_deactivate_primary_element();
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
                {
                    g_card_step = g_carda_steps_write_alt_save;
                }
                else
                {
                    g_card_step = g_carda_steps_write_save_keep_handles;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "Overwrite data?" prompt for an existing save file and
 *        handle its input; "yes" starts writing the save file.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_overwrite_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* prompt;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_overwrite_prompt, CARD_MENU_TEXT_OVERWRITE_PROMPT),
                                                      FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        carda_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            carda_deactivate_primary_element();
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                carda_deactivate_primary_element();
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
                {
                    g_card_step = g_carda_steps_scan_and_write_alt_save;
                }
                else
                {
                    g_card_step = g_carda_steps_write_save;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "Now Saving..." message and progress bar; once the write
 *        has finished, replace the window with the "Saved." message.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_save_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 x;
    void* result;
    u16* text_table;
    CardMenuElement* message;
    s32 unused[2];

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_saving, CARD_MENU_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_carda_text_saving, CARD_MENU_TEXT_SAVING);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_save_in_progress == 0)
    {
        field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        message = g_carda_element_pool;
        message->draw = carda_draw_save_complete;
        message->attr.bits.transition_step = 1;
        message->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        message->attr.bits.x = CARD_MENU_MESSAGE_X;
        message->attr.bits.y = 0x68;
        message->size.bits.width_high = 1;
        message->size.bits.height = 0x10;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(message, 0x20);
    }
    return result;
}

/**
 * @brief Draw the "Saved." message and close it on confirm, cancel or entry
 *        a missing card.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_save_complete(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;

    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_saved, CARD_MENU_TEXT_SAVED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_MESSAGE_WIDTH / 2,
                             -y_offset, FIELD_TEXT_ALIGN_CENTER);
    if (g_pad_input & (CARD_MENU_CONFIRM_BUTTON_MASK | PAD_BTN_CIRCLE))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        carda_deactivate_primary_element();
        field_reset_input_repeat();
    }
    else if (g_card_entry_state == CARD_MENU_ENTRY_STATE_NO_CARD)
    {
        carda_deactivate_primary_element();
        field_reset_input_repeat();
    }
    return result;
}

/**
 * @brief Draw the "Format?" prompt for an unformatted memory card and handle
 *        its input; "yes" replaces the window with the formatting message.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note JP names the card type on the first line and puts "not formatted" on the
 *       second line for both card types.
 */
void* carda_draw_format_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 x;
    s32 status;
    u32 attr;
    u16* text_table;
    CardMenuElement* message;
    s32 y;

    y = y_offset;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_pocketstation_is, CARD_MENU_TEXT_POCKETSTATION_IS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#if !defined(VERSION_JP)
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_pocketstation_is, CARD_MENU_TEXT_POCKETSTATION_IS);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NOT_FORMATTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_MESSAGE_WIDTH / 2,
                               CARD_MENU_LINE_HEIGHT - y, FIELD_TEXT_ALIGN_CENTER);
#endif
    }
    else
    {
#if defined(VERSION_JP)
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_memory_card_is, CARD_MENU_TEXT_MEMORY_CARD_IS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#else
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_unformatted, CARD_MENU_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#endif
    }
    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
#if defined(VERSION_JP)
    prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_unformatted, CARD_MENU_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL, x,
                           CARD_MENU_LINE_HEIGHT - y, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_carda_text_card_unformatted, CARD_MENU_TEXT_CARD_UNFORMATTED);
    prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_FORMAT_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                           CARD_MENU_LINE_HEIGHT * 2 - y, FIELD_TEXT_ALIGN_CENTER);
#else
    prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_format_prompt, CARD_MENU_TEXT_FORMAT_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                           CARD_MENU_LINE_HEIGHT * 2 - y, FIELD_TEXT_ALIGN_CENTER);
#endif
    prim = carda_draw_choice_prompt(prim, ot, x, CARD_MENU_LINE_HEIGHT * 3 - y);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        carda_deactivate_primary_element();
        field_reset_input_repeat();
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_card_step = g_carda_steps_card_reset;
        }
        else
        {
            g_card_step = NULL;
        }
    }
    else
    {
        status = g_pad_input;
        if ((status & PAD_BTN_CIRCLE) || ((status & CARD_MENU_CONFIRM_BUTTON_MASK) && g_carda_choice_toggle != 0))
        {
            g_carda_format_declined = 1;
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            carda_deactivate_primary_element();
            field_reset_input_repeat();
            g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
            carda_reset_entry_ranks();
            g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
            if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
            {
                g_card_step = NULL;
            }
            else
            {
                g_card_step = g_carda_steps_card_reset;
            }
        }
        else if (status & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            carda_deactivate_primary_element();
            carda_reset_entry_ranks();
            g_carda_format_frames = 0;
            message = g_carda_element_pool;
            message->attr.bits.transition_step = 1;
            message->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
            if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
            {
                message->attr.bits.x = CARD_MENU_MESSAGE_X;
                message->attr.bits.y = CARDA_TRANSFER_Y;
                attr = message->attr.word;
                attr &= (1 << CARD_MENU_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARD_MENU_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.bits.width_high = 1;
                message->size.bits.height = CARDA_TRANSFER_HEIGHT;
            }
            else
            {
                message->attr.bits.x = CARD_MENU_MESSAGE_X;
                message->attr.bits.y = CARDA_MESSAGE_Y;
                attr = message->attr.word;
                attr &= (1 << CARD_MENU_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARD_MENU_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.bits.width_high = 1;
                message->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
            }
            message->draw = carda_draw_format_progress;
        }
    }
    return prim;
}

/**
 * @brief Draw the formatting message (or, once done, the saving message);
 *        on frame 12 format the card, on frame 13 build the save file and
 *        switch to the saving window.
 * @param ot Ordering table used for the text primitives.
 * @param prim Current primitive packet cursor.
 * @param x_offset Horizontal placement offset.
 * @param y_offset Vertical placement offset.
 * @return Updated primitive packet cursor.
 */
static void* carda_draw_format_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];

    if (g_carda_format_frames >= 13)
    {
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_downloading, CARD_MENU_TEXT_DOWNLOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_downloading, CARD_MENU_TEXT_DOWNLOADING);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_saving, CARD_MENU_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_saving, CARD_MENU_TEXT_SAVING);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }
    else
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_formatting, CARD_MENU_TEXT_FORMATTING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_formatting, CARD_MENU_TEXT_FORMATTING);
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
        else
        {
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }

    if (g_carda_format_frames == 12)
    {
        carda_reset_to_new_save_entry();
        g_card_step = g_carda_steps_card_check;
    }
    else if (g_carda_format_frames >= 13)
    {
        g_carda_selected_row = 0;
        g_card_entry_suffix_values[0] = 1;
        carda_build_save_file();
        g_carda_progress_bar_active = 0;
        g_carda_save_in_progress = 1;
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_carda_received_item_count = 0;
            g_gosub_result_values = 5;
            g_carda_new_save_file = 1;
            carda_store_active_record();
            g_card_step = g_carda_steps_write_alt_save;
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOADING;
            carda_deactivate_primary_element();
        }
        else
        {
            CardMenuElement* message;

            g_card_step = g_carda_steps_write_save_keep_handles;
            message = g_carda_element_pool;
            message->draw = carda_draw_save_progress;
            message->attr.bits.transition_step = 1;
            message->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
            message->attr.bits.x = CARD_MENU_MESSAGE_X;
            message->attr.bits.y = CARDA_MESSAGE_Y;
            message->size.bits.width_high = 1;
            message->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
            CARD_MENU_SET_ELEMENT_WIDTH_LOW(message, 0x20);
        }
    }
    g_carda_format_frames += 1;
    return prim;
}

/**
 * @brief Open the status dialog in the first element slot and abandon any
 *        card operation in progress; in overlay modes 2 and 3 only the entry
 *        state is set.
 * @param dialog_state Message to show, from the CARDA_DIALOG values.
 */
void carda_open_status_dialog(s32 dialog_state)
{
    s32 state;
    CardMenuElement* dialog;

    if (g_carda_dialog_state != dialog_state || g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state == CARD_MENU_ELEMENT_FREE ||
        g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].draw != carda_draw_status_dialog)
    {
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = CARD_MENU_SELECTION_NONE;
        g_carda_io_busy = 0;
        carda_reset_entry_ranks();
        g_carda_dialog_state = dialog_state;
        _card_wait(g_card_slot);
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);

        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            state = g_carda_dialog_state;
            switch (state)
            {
            case CARD_MENU_DIALOG_SAVE_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOAD_FAILED;
                break;
            case CARD_MENU_DIALOG_LOAD_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_UPLOAD_FAILED;
                break;
            case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
                g_card_entry_state = CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED;
                break;
            case CARD_MENU_DIALOG_NOT_POCKETSTATION:
                g_card_entry_state = CARDA_ENTRY_STATE_NO_POCKETSTATION;
                break;
            case CARDA_DIALOG_SAVE_CORRUPT:
                g_card_entry_state = CARDA_ENTRY_STATE_SAVE_CORRUPT;
                break;
            case CARDA_DIALOG_FORMAT_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_FORMAT_FAILED;
                break;
            }
            g_card_step = NULL;
            return;
        }

        dialog = g_carda_element_pool;
        dialog->attr.bits.transition_step = 1;
        dialog->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        dialog->attr.bits.x = CARD_MENU_DIALOG_X;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(dialog, CARD_MENU_DIALOG_WIDTH);
        dialog->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
        if (dialog_state < 2 || dialog_state == 4 || dialog_state == 5)
        {
            dialog->attr.bits.y = 0x60;
            dialog->size.bits.height = 0x24;
        }
        else
        {
            dialog->size.bits.height = 0x14;
            dialog->attr.bits.y = 0x70;
        }
        dialog->draw = carda_draw_status_dialog;
        field_reset_input_repeat();
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = CARD_MENU_SELECTION_NONE;
        g_carda_io_busy = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = g_carda_steps_initial_scan;
        g_carda_dialog_state = dialog_state;
        _card_wait(g_card_slot);
    }
}

/**
 * @brief Draw the active CARDA status dialog and handle dismissal input.
 * @param ot Ordering table used by the text renderer.
 * @param prim Current primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    u16* text_table;

    switch (g_carda_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        if (g_card_entry_state == CARD_MENU_ENTRY_STATE_NO_CARD)
        {
            carda_deactivate_primary_element();
            field_reset_input_repeat();
            return prim;
        }
        break;
    case CARDA_DIALOG_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_SAVE_CORRUPT), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_FORMAT_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_format_failed, CARD_MENU_TEXT_FORMAT_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_format_failed, CARD_MENU_TEXT_FORMAT_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = CARDA_RESULT_SEND_PET_CANCELLED;
            break;
        case 3:
            g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
            break;
        default:
            g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
            break;
        }
        carda_deactivate_primary_element();
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Upload one save icon's CLUT and pixels to VRAM slot @p index and draw
 *        it as a 48x48 textured quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @param x Quad left edge.
 * @param y Quad top edge.
 * @param width Quad width.
 * @param icon Icon id; SAVE_NO_ICON draws nothing, ids below 2 in row 1 and ids from
 *             0x4F up get a generated CLUT (field_copy_portrait_palette / field_copy_golem_portrait_palette).
 * @param index VRAM icon slot; selects the CLUT row entry and the texture column.
 * @param row Entry row; row 1 uses the generated CLUT for icons 0 and 1.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged for SAVE_NO_ICON.
 */
void* carda_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 column;
    u8 u;

    if (icon == SAVE_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y, GPU_CLUT_4BIT_COLORS, 1);
    if ((row == 1) && (icon < 2))
    {
        field_copy_portrait_palette(g_carda_icon_context, icon);
        LoadImage(&rect, (u_long*)g_carda_icon_context);
        DrawSync(0);
    }
    else if (icon >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_carda_icon_context, g_carda_icon_palette);
        LoadImage(&rect, (u_long*)g_carda_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_carda_icon_image_offsets, icon)->clut);
    }

    column = index * 3;
    setRECT(&rect, column * 4 + CARD_MENU_ICON_VRAM_X, CARD_MENU_ICON_VRAM_Y, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
    LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_carda_icon_image_offsets, icon)->pixels);

    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    u = column * 16;
    quad->u2 = u;
    quad->u0 = u;
    u += FIELD_PORTRAIT_SIZE - 1;
    quad->u3 = u;
    quad->u1 = u;
    quad->v1 = CARD_MENU_ICON_VRAM_Y;
    quad->v0 = CARD_MENU_ICON_VRAM_Y;
    quad->x1 = x + width;
    quad->y3 = y + FIELD_PORTRAIT_SIZE - 1;
    quad->y2 = y + FIELD_PORTRAIT_SIZE - 1;
    quad->v3 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->v2 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->clut = getClut(index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y);
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, CARD_MENU_ICON_VRAM_X, 0);
    addPrim(ot, quad);

    return quad + 1;
}

/**
 * @brief Preselect the default choice (CARD_MENU_CHOICE_DEFAULT) of the two-choice prompt.
 */
inline void carda_enable_choice_toggle(void)
{
    g_carda_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
}

/**
 * @brief Draw the two choices of a yes/no prompt from the FIELD UI string table,
 *        highlighting the selected one, and toggle the selection on left/right.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the text is linked into.
 * @param x Prompt center; the choices are drawn CARD_MENU_CHOICE_YES_GAP left and CARD_MENU_CHOICE_NO_GAP right of it.
 * @param y Prompt baseline.
 * @return Advanced primitive-buffer cursor.
 */
static inline void* carda_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* yes_text;
    u8* no_text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    yes_text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = FIELD_UI_TEXT_TABLE(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    if (g_carda_choice_toggle != 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, yes_text, color, x - CARD_MENU_CHOICE_YES_GAP, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    no_text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_carda_choice_toggle == 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, no_text, color, x + CARD_MENU_CHOICE_NO_GAP, y, FIELD_TEXT_ALIGN_LEFT);

    if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
    {
        g_carda_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Draw the two-line notice shared by the mode 3 error states (texts 0x41 and 0x42).
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @param second_y Baseline of the second line, before the transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static inline void* carda_draw_mode3_notice(void* prim, u_long* ot, s32 x_offset, s32 y_offset, s32 second_y)
{
    s32 x;
    u16* text_table;

    x = -x_offset + CARDA_SAVE_TEXT_X;
    prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_ring_ring_land_was, CARD_MENU_TEXT_RING_RING_LAND_WAS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                           FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_carda_text_ring_ring_land_was, CARD_MENU_TEXT_RING_RING_LAND_WAS);
    return field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NOT_FOUND), FIELD_TEXT_COLOR_NORMAL, x, second_y - y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the mode 2/3 save window and run its save flow.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Dispatches on g_card_entry_state: below CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH
 *       it holds the directory entry count while the card is searched for an existing
 *       save; above it, the dialog currently shown. Dialog states also read the pad,
 *       move to the next state and start card sequences through g_card_step.
 */
void* carda_draw_save_flow(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 counter;

    if (g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state != CARD_MENU_ELEMENT_FREE)
    {
        /* The message states draw nothing while element 0 is active. */
        switch (g_card_entry_state)
        {
        case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
        case CARDA_ENTRY_STATE_FORMAT_FAILED:
        case CARDA_ENTRY_STATE_SAVE_CORRUPT:
        case CARDA_ENTRY_STATE_NO_POCKETSTATION:
        case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
        case CARDA_ENTRY_STATE_UPLOAD_FAILED:
        case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
        case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        case CARD_MENU_ENTRY_STATE_CARD_FULL:
        case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        case CARD_MENU_ENTRY_STATE_NO_CARD:
        case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
            return prim;
        }
    }

    switch (g_card_entry_state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARD_MENU_LINE_HEIGHT);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        break;

    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARD_MENU_LINE_HEIGHT);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        }
        else
        {
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_unformatted, CARD_MENU_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                                   -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_checking_pocketstation, CARD_MENU_TEXT_CHECKING_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_checking_pocketstation, CARD_MENU_TEXT_CHECKING_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }

    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARD_MENU_LINE_HEIGHT);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
            g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DOWNLOAD_RING_RING_LAND), FIELD_TEXT_COLOR_NORMAL, x, 0x10 - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_NEEDS_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, 0x20 - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                g_carda_choice_toggle = g_card_slot;
                field_reset_input_repeat();
            }
        }
        break;

    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_no_pocketstation, CARD_MENU_TEXT_NO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_pocketstation_access_failed, CARD_MENU_TEXT_POCKETSTATION_ACCESS_FAILED),
                               FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;

    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARD_MENU_LINE_HEIGHT);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_RING_RING_LAND:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
        break;

    case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_download_failed, CARD_MENU_TEXT_DOWNLOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_UPLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_upload_failed, CARD_MENU_TEXT_UPLOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_pocketstation_not_inserted, CARD_MENU_TEXT_POCKETSTATION_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_no_pocketstation, CARD_MENU_TEXT_NO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_save_corrupt, CARD_MENU_TEXT_SAVE_CORRUPT), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_FORMAT_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_format_failed, CARD_MENU_TEXT_FORMAT_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_pet_already_on_ranch, CARD_MENU_TEXT_PET_ALREADY_ON_RANCH), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARD_MENU_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_CONFIRM_RETURN:
    {
        s32 x;
        u16* text_table;

        x = -x_offset;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_return_pet, CARD_MENU_TEXT_RETURN_PET), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_return_pet, CARD_MENU_TEXT_RETURN_PET);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_RING_RING_LAND_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_WILL_BE_ERASED), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_choice_prompt(prim, ot, x + CARDA_SAVE_TEXT_X, CARD_MENU_LINE_HEIGHT * 3 - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_carda_choice_toggle = g_card_slot;
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            field_reset_input_repeat();
            break;
        }
        if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                g_carda_choice_toggle = g_card_slot;
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                field_reset_input_repeat();
                break;
            }
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            g_carda_new_save_file = 0;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_active = 1;
            g_card_step = g_carda_steps_read_save_prefix;
            g_card_entry_state = CARDA_ENTRY_STATE_UPLOADING;
        }
        break;
    }

    case CARDA_ENTRY_STATE_UPLOADING:
    {
        s32 i;
        s32 x;
        s32 save_id;
        u16* text_table;
        CardMenuElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_game_from_pocketstation, CARD_MENU_TEXT_GAME_FROM_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_game_from_pocketstation, CARD_MENU_TEXT_GAME_FROM_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_UPLOADING), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT * 3 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_progress_active != 0)
        {
            break;
        }
        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD;
        g_carda_pet_already_on_ranch = 0;
        save_id = CARDA_SAVE_DATA->record.unique_id;
        for (i = 0; i < PET_RECORD_COUNT; i++)
        {
            if (g_saved_game_ctx->pets[i].name[0] != 0 && g_saved_game_ctx->pets[i].unique_id == save_id)
            {
                g_carda_pet_already_on_ranch = 1;
                break;
            }
        }
        if (g_carda_pet_already_on_ranch != 0)
        {
            g_card_entry_state = CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH;
            g_card_step = NULL;
            break;
        }
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            for (g_field_card_pet_slot = 0; g_field_card_pet_slot < PET_RECORD_COUNT; g_field_card_pet_slot++)
            {
                if (g_saved_game_ctx->pets[g_field_card_pet_slot].name[0] == 0)
                {
                    break;
                }
            }
            g_field_card_overlay_mode = CARDA_RESULT_PET_RETURNED;
            if (g_field_card_pet_slot == PET_RECORD_COUNT)
            {
                g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
                carda_clear_elements();
                field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
                break;
            }
            carda_apply_save_items();
            carda_restore_active_record();
            strcpy(g_carda_selected_card_path, CARD_DEVICE_PREFIX);
            g_gosub_result_values = g_field_card_pet_slot;
            g_carda_selected_card_path[CARD_DEVICE_SLOT_DIGIT] += (u8)g_card_slot;
            strcat(g_carda_selected_card_path, g_lom_pocketstation_filename_prefix);
            _card_wait(g_card_slot);
            erase(g_carda_selected_card_path);
            if (g_carda_received_item_count == 0)
            {
                CardMenuElement* element;

                g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
                element = g_carda_element_pool;
                for (counter = 0; counter < CARD_MENU_ELEMENT_COUNT; counter++)
                {
                    element->attr.bits.state = CARD_MENU_ELEMENT_FREE;
                    element++;
                }
                field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
                break;
            }
            carda_open_item_list();
            break;
        }
        carda_enable_choice_toggle();
        field_reset_input_repeat();
        break;
    }

    case CARDA_ENTRY_STATE_SELECT_SLOT:
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, -y_offset);
        break;

    case CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD:
    {
        if (g_carda_new_save_file != 0)
        {
            s32 x;
            u16* text_table;

            x = -x_offset;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_game_to_pocketstation, CARD_MENU_TEXT_GAME_TO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                                   x + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_game_to_pocketstation, CARD_MENU_TEXT_GAME_TO_POCKETSTATION);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DOWNLOAD_OK), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_choice_prompt(prim, ot, x + CARDA_SAVE_TEXT_X, CARD_MENU_LINE_HEIGHT * 2 - y_offset);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_swap_pets, CARD_MENU_TEXT_SWAP_PETS), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   -y_offset, FIELD_TEXT_ALIGN_CENTER);
            text_table = CARD_MENU_TEXT_TABLE(g_carda_text_swap_pets, CARD_MENU_TEXT_SWAP_PETS);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_GAME_TO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_OVERWRITE_OK), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_choice_prompt(prim, ot, x + CARDA_SAVE_TEXT_X, CARD_MENU_LINE_HEIGHT * 3 - y_offset);
        }
        if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && g_carda_choice_toggle != 0))
        {
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            g_carda_choice_toggle = g_card_slot;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            g_carda_received_item_count = 0;
            if (g_carda_new_save_file == 0)
            {
                carda_apply_save_items();
                g_gosub_result_values = g_field_card_pet_slot;
            }
            else
            {
                g_gosub_result_values = PET_RECORD_COUNT;
            }
            carda_store_active_record();
            g_carda_save_in_progress = 1;
            g_card_step = g_carda_steps_overwrite_alt_save;
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOADING;
        }
        break;
    }

    case CARDA_ENTRY_STATE_DOWNLOADING:
    {
        s32 x;
        s32 i;
        u16* text_table;
        CardMenuElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_downloading, CARD_MENU_TEXT_DOWNLOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_downloading, CARD_MENU_TEXT_DOWNLOADING);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_save_in_progress == 0)
        {
            g_field_card_overlay_mode = CARDA_RESULT_PET_SENT;
            field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
            if (g_gosub_result_values == PET_RECORD_COUNT)
            {
                g_saved_game_ctx->pets[g_field_card_pet_slot].name[0] = 0;
            }
            else
            {
                carda_restore_active_record();
            }
            if (g_carda_received_item_count != 0)
            {
                carda_open_item_list();
                break;
            }
            carda_clear_elements();
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        }
        break;
    }

    default:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_checking_pocketstation, CARD_MENU_TEXT_CHECKING_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_carda_text_checking_pocketstation, CARD_MENU_TEXT_CHECKING_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        /* Search one entry per frame, unless a directory scan (opcode 6 or 7) is running. */
        if (g_carda_entry_scan_active == 0 && g_carda_io_busy == 0 && (u32)(*g_card_step - CARD_MENU_STEP_SCAN_ENTRIES) >= 2U)
        {
            if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0)
            {
                s32 row_y;
                s32 delta;

                g_carda_selected_row++;
                if (g_carda_selected_row >= g_card_entry_state)
                {
                    if (g_carda_mode == CARDA_MODE_RETURN_PET)
                    {
                        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                        break;
                    }
                    strcpy(g_carda_selected_card_path, CARD_DEVICE_PREFIX);
                    g_carda_new_save_file = 1;
                    g_carda_selected_card_path[CARD_DEVICE_SLOT_DIGIT] += (u8)g_card_slot;
                    strcat(g_carda_selected_card_path, g_lom_pocketstation_filename_prefix);
                    carda_store_active_record();
                    g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD;
                    carda_enable_choice_toggle();
                    field_reset_input_repeat();
                    break;
                }
                carda_commit_selected_entry();
                carda_scroll_to_selection();
                break;
            }
            if (g_carda_mode == CARDA_MODE_RETURN_PET)
            {
                carda_enable_choice_toggle();
                g_carda_new_save_file = 0;
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_active = 1;
                g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_RETURN;
            }
            else
            {
                g_carda_new_save_file = 0;
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_active = 1;
                g_card_step = g_carda_steps_read_save_prefix;
                g_card_entry_state = CARDA_ENTRY_STATE_UPLOADING;
            }
        }
        break;
    }

    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    }
    return prim;
}

/**
 * @brief Serialize the game state into the save buffer and copy the active record into it.
 */
void carda_store_active_record(void)
{
    cdrom_queue_read(0x5E2, g_carda_save_blob);
    cdrom_wait_queue_empty();
    card_prepare_pet_transfer(g_carda_save_blob, &g_saved_game_ctx->pets[g_field_card_pet_slot]);
}

/**
 * @brief Restore the active record from g_carda_saved_record_copy and apply the save's growth delta.
 */
static void carda_restore_active_record(void)
{
    bcopy(g_carda_saved_record_copy, (u8*)&g_saved_game_ctx->pets[g_field_card_pet_slot], sizeof(PetRecord));
    g_saved_game_ctx->pets[g_field_card_pet_slot].progress.bits.experience += g_carda_growth_delta;
    field_apply_region_level_ups(g_field_card_pet_slot);
}

/**
 * @brief Draw the card-slot prompt and handle switch, cancel and retry input.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @return GPU packet cursor after the prompt text.
 */
static void* carda_draw_slot_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    CardMenuElement* element;
    void* result;
    s32 i;

    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_select_slot, CARD_MENU_TEXT_SELECT_SLOT), FIELD_TEXT_COLOR_NORMAL, x, y, FIELD_TEXT_ALIGN_CENTER);

    if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
    {
        g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
        g_card_slot ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        return result;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = CARDA_RESULT_SEND_PET_CANCELLED;
            break;
        case 3:
            g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
            break;
        default:
            g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
            break;
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        carda_close_all_elements();
        return result;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        s32 slot;

        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        slot = g_card_slot;
        g_carda_format_declined = 0;
        g_card_step = NULL;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_carda_scroll_frames = 0;
        g_carda_scroll_target_y = 0;
        g_carda_scroll_y = 0;
        g_carda_selected_row = 0;
        g_card_slot ^= 1;
        g_carda_selection_status = CARD_MENU_SELECTION_NONE;
        /* Same reset as switching cards, but stay on the current slot. */
        g_card_slot = slot;
        carda_reset_entry_ranks();
        clear_hardware_card_events();
        clear_software_card_events();
        g_carda_progress_bar_active = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_card_step = g_carda_steps_initial_scan;
    }

    return result;
}

/**
 * @brief Reset the CARDA choice state and initialize its mode-specific element.
 * @param dialog_state Status-dialog state index.
 */
void carda_open_save_status_dialog(s32 dialog_state)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    field_reset_input_repeat();
    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = CARD_MENU_SELECTION_NONE;
    g_carda_io_busy = 0;
    carda_reset_entry_ranks();
    g_card_step = NULL;
    g_carda_dialog_state = dialog_state;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        switch (dialog_state)
        {
        case CARD_MENU_DIALOG_SAVE_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOAD_FAILED;
            break;
        case CARD_MENU_DIALOG_LOAD_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_UPLOAD_FAILED;
            break;
        case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
            g_card_entry_state = CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED;
            break;
        case CARD_MENU_DIALOG_NOT_POCKETSTATION:
            g_card_entry_state = CARDA_ENTRY_STATE_NO_POCKETSTATION;
            break;
        case CARDA_DIALOG_SAVE_CORRUPT:
            g_card_entry_state = CARDA_ENTRY_STATE_SAVE_CORRUPT;
            break;
        case CARDA_DIALOG_FORMAT_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_FORMAT_FAILED;
            break;
        }
        g_card_step = NULL;
        return;
    }

    g_carda_element1_state.draw = carda_draw_save_status_dialog;
    g_carda_element1_state.attr.bits.transition_step = 1;
    g_carda_element1_state.attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    g_carda_element1_state.attr.bits.x = 0x20;
    g_carda_element1_state.attr.bits.y = 0x70;
    g_carda_element1_state.size.bits.width_high = 1;
    g_carda_element1_state.size.bits.height = 0x14;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(&g_carda_element1_state, 0);
}

/**
 * @brief Draw the status message chosen by g_carda_dialog_state and close the menu on confirm.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the message.
 */
static void* carda_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];

    switch (g_carda_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
    case CARDA_DIALOG_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        carda_clear_elements();
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Clear the element pool and open the item list and its header.
 */
static void carda_open_item_list(void)
{
    CardMenuElement* element;

    carda_clear_elements();

    g_carda_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element = carda_alloc_element();
    element->draw = carda_draw_item_list;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = CARDA_ITEM_LIST_Y;
    element->size.bits.width_high = 1;
    element->size.bits.height = CARDA_ITEM_LIST_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, 0);

    element = carda_alloc_element();
    element->draw = carda_draw_item_list_header;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = 0x1A;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x10;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, 0);
    g_carda_scroll_y = 0;
    carda_deactivate_primary_element();
}

/**
 * @brief Draw the item-list header text.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the text.
 */
static void* carda_draw_item_list_header(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_carda_text_received_items, CARD_MENU_TEXT_RECEIVED_ITEMS), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the visible rows of the received-item list and handle scrolling.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the list.
 */
void* carda_draw_item_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 row_y;
    s32 i;
    void* result;

    result = prim;
    for (i = 0; i < g_carda_received_item_count; i++)
    {
        row_y = i * CARD_MENU_LINE_HEIGHT - g_carda_scroll_y;
        if ((u32)(row_y + 13) < 157)
        {
            result = field_draw_text(result, ot, (u8*)g_carda_item_names + g_carda_item_names[g_carda_received_item_ids[i]], FIELD_TEXT_COLOR_NORMAL,
                                     -x_offset + CARDA_ITEM_LIST_WIDTH / 2, row_y - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }

    if (g_carda_scroll_frames == 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            if (g_carda_scroll_y != 0)
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                g_carda_scroll_frames = CARD_MENU_SCROLL_FRAMES;
                g_carda_scroll_target_y -= CARD_MENU_LINE_HEIGHT;
            }
        }
        else if (g_pad_input & PAD_BTN_DOWN)
        {
            if ((g_carda_received_item_count * CARD_MENU_LINE_HEIGHT - g_carda_scroll_y) >= 141)
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                g_carda_scroll_frames = CARD_MENU_SCROLL_FRAMES;
                g_carda_scroll_target_y += CARD_MENU_LINE_HEIGHT;
            }
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            carda_clear_elements();
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        }
    }
    return result;
}

/**
 * @brief Copy the save's record into g_carda_saved_record_copy and add the save's items to the game state.
 * @note Items already at CARDA_ITEM_COUNT_MAX are skipped; the ids that were added
 *       are listed in g_carda_received_item_ids and counted in g_carda_received_item_count.
 */
static void carda_apply_save_items(void)
{
    CardaSaveData* save;
    CardaSaveItemList* list;
    u32 i;
    u8 count;

    save = CARDA_SAVE_DATA;
    list = &save->items;
    g_carda_received_item_count = 0;
    bcopy((u8*)&save->record, g_carda_saved_record_copy, sizeof(PetRecord));
    g_carda_received_item_count = 0;
    g_carda_growth_delta = list->growth_delta;
    for (i = 0; i < list->count; i++)
    {
        count = g_saved_game_ctx->item_counts[list->ids[i]];
        if (count < CARDA_ITEM_COUNT_MAX)
        {
            g_saved_game_ctx->item_counts[list->ids[i]] = count + 1;
            g_carda_received_item_ids[g_carda_received_item_count] = list->ids[i];
            g_carda_received_item_count++;
        }
    }
}
