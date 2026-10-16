#include "internal/niki_internal.h"
#include "overlays/menu/menu.h"
#include <fcntl.h>

#include "../../common/sjis/format_decimal.inc.c"
#include "../../common/save_file/format_hex.inc.c"
#include "../../common/save_file/hex_nibble_to_ascii.inc.c"
#include "../../common/save_file/parse_hex.inc.c"
#include "../../common/save_file/parse_hex_suffix_byte.inc.c"
#include "../../common/card_directory/parse_entry_fields.inc.c"

#include "../../common/card_directory/card_rank_entries.inc.c"
#include "../../common/card_directory/card_reset_entry_ranks.inc.c"
#include "../../common/card_directory/card_has_known_entry_type.inc.c"
#include "../../common/card_directory/card_entry_blocks_reach_limit.inc.c"
#include "../../common/card_directory/card_erase_placeholder_files.inc.c"

/**
 * @brief Execute the current memory-card load/save command and advance its sequence.
 * @return Command result reported by the current sequence step.
 */
s32 niki_advance_load_sequence(void)
{
    CardSequencePath path;
    long card_command;
    long card_result;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 io_result;
    s32 command;

    strcpy(path.text, CARD_DEVICE_PREFIX);
    phase_result = CARD_MENU_SEQUENCE_WAIT;
    path.device.characters.slot += (u8)g_card_slot;

    if (g_card_step == NULL)
    {
        return phase_result;
    }

    command = *g_card_step;
    switch (command)
    {
    case CARD_MENU_STEP_CARD_INFO:
        phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
        _card_wait(g_card_slot);
        _card_info(CARD_CHANNEL(g_card_slot));
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_POLL_CARD_INFO:
        poll_result = poll_software_card_events();
        switch (poll_result)
        {
        case 1:
        case 2:
            phase_result = CARD_MENU_SEQUENCE_NO_CARD;
            g_card_menu_selection_status = 0;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
            g_card_step = g_card_step + 1;
            break;
        case 3:
            card_reset_entry_ranks();
            g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
            g_card_step = g_card_steps_initial_scan;
            break;
        case 0:
            g_card_step = g_card_step + 1;
            break;
        default:
            return phase_result;
        }
        break;

    case CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS:
        clear_software_card_events();
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_POLL_HARDWARE_EVENTS:
        while ((poll_result = poll_hardware_card_events()) == CARD_EVENT_NONE);
        if (poll_result == 0)
        {
            g_card_step = g_card_step + 1;
            break;
        }
        if (poll_result < 0)
        {
            return phase_result;
        }
        if (poll_result >= 4)
        {
            return phase_result;
        }
        phase_result = CARD_MENU_SEQUENCE_NO_CARD;
        g_card_menu_selection_status = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
        break;

    case CARD_MENU_STEP_CLEAR_HARDWARE_EVENTS:
        clear_hardware_card_events();
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_SCAN_ENTRIES:
        card_erase_placeholder_files();
        g_card_menu_entry_scan_active = 1;
        if (card_menu_begin_entry_scan(g_card_slot) == 0)
        {
            phase_result = CARD_MENU_SEQUENCE_FINISHED;
            g_card_step = NULL;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
            g_card_menu_entry_scan_active = 0;
            break;
        }
        g_card_step = g_card_step + 1;
        for (wait_attempts = 0; wait_attempts < CARD_MENU_FILE_OP_ATTEMPTS; wait_attempts++)
        {
            if (niki_scan_next_entry(g_card_slot) == 0)
            {
                if (g_card_menu_mode != 0)
                {
                    g_card_menu_selected_row = 0;
                }
                g_card_menu_entry_scan_active = 0;
                if (g_card_entry_state == CARD_MENU_ENTRY_STATE_NO_GAME_DATA)
                {
                    return phase_result;
                }
                if (g_card_entry_state == CARD_MENU_ENTRY_STATE_CARD_FULL)
                {
                    break;
                }
                card_menu_commit_selected_entry();
                break;
            }
        }
        break;

    case CARD_MENU_STEP_CLEAR_CARD:
        phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
        _card_wait(g_card_slot);
        _card_clear(CARD_CHANNEL(g_card_slot));
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_LOAD_CARD:
        phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
        _card_wait(g_card_slot);
        _card_load(CARD_CHANNEL(g_card_slot));
        g_card_menu_new_card_retries = CARD_MENU_CARD_LOAD_RETRIES;
        g_card_menu_load_error_retries = CARD_MENU_CARD_LOAD_RETRIES;
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_DONE:
        phase_result = CARD_MENU_SEQUENCE_FINISHED;
        g_card_menu_write_in_progress = 0;
        break;

    case CARD_MENU_EXCHANGE_STEP_ERASE_ENTRY:
        strcat(path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
        wait_attempts = 0;
        _card_wait(g_card_slot);
        while (wait_attempts < CARD_MENU_FILE_OP_ATTEMPTS)
        {
            poll_result = erase(path.text);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        }
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_POLL_CARD_LOAD:
        poll_result = poll_software_card_events();
        switch (poll_result)
        {
        case 1:
        case 2:
            g_card_menu_load_error_retries = g_card_menu_load_error_retries - 1;
            if (g_card_menu_load_error_retries == 0)
            {
                phase_result = CARD_MENU_SEQUENCE_NO_CARD;
                g_card_menu_selection_status = 0;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                break;
            }
            _card_wait(g_card_slot);
            _card_clear(CARD_CHANNEL(g_card_slot));
            _card_wait(g_card_slot);
            _card_load(CARD_CHANNEL(g_card_slot));
            break;
        case 3:
            g_card_menu_new_card_retries = g_card_menu_new_card_retries - 1;
            if (g_card_menu_new_card_retries != 0)
            {
                _card_wait(g_card_slot);
                _card_clear(CARD_CHANNEL(g_card_slot));
                _card_wait(g_card_slot);
                _card_load(CARD_CHANNEL(g_card_slot));
                break;
            }
            phase_result = CARD_MENU_SEQUENCE_UNFORMATTED;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_SAVE_DATA;
            g_card_step = g_card_steps_idle;
            break;
        case 0:
            g_card_step = g_card_step + 1;
            break;
        }
        break;

    case CARD_MENU_STEP_WAIT_HARDWARE_EVENTS:
        while ((poll_result = poll_hardware_card_events()) == CARD_EVENT_NONE);
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_READ_ENTRY:
        g_card_menu_io_busy = 1;
        g_card_menu_selection_status = 0;
        _card_wait(g_card_slot);
        g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
        if (g_card_menu_file_handle == CARD_FILE_ERROR)
        {
            break;
        }
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (read(g_card_menu_file_handle, &g_niki_entry_file, g_card_menu_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : CARD_MENU_ENTRY_TITLE_READ_BYTES) == CARD_FILE_ERROR)
        {
            close(g_card_menu_file_handle);
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_POLL_ENTRY_READ:
        poll_result = poll_software_card_events();
        if (poll_result == 0)
        {
            g_card_menu_io_busy = 0;
            g_card_menu_selection_status = 1;
            g_card_step = g_card_step + 1;
            close(g_card_menu_file_handle);
            break;
        }
        if (poll_result == CARD_EVENT_NONE)
        {
            break;
        }
        g_card_menu_io_busy = 0;
        close(g_card_menu_file_handle);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_card_step = g_card_steps_initial_scan;
        break;

    case CARD_MENU_STEP_READ_SAVE:
        g_card_menu_progress_active = 1;
        g_card_menu_progress_bar_active = 1;
        g_card_menu_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (read(g_card_menu_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == CARD_FILE_ERROR)
        {
            g_card_menu_retry_count = g_card_menu_retry_count - 1;
            if (g_card_menu_retry_count == 0)
            {
                card_menu_open_status_dialog(1);
                break;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_STEP_POLL_SAVE_READ:
        io_result = poll_software_card_events();
        if (io_result == 0)
        {
            g_card_menu_progress_active = 0;
            g_card_step = g_card_step + 1;
            close(g_card_menu_file_handle);
            break;
        }
        if (io_result < 0)
        {
            break;
        }
        if (io_result >= 4)
        {
            break;
        }
        g_card_menu_retry_count = g_card_menu_retry_count - 1;
        if (g_card_menu_retry_count == 0)
        {
            g_card_menu_progress_bar_active = 0;
            card_menu_open_status_dialog(1);
            return phase_result;
        }
        g_card_step = g_card_step - 1;
        break;

    case CARD_MENU_STEP_CHECK_POCKETSTATION:
        for (wait_attempts = 0; wait_attempts < CARD_MENU_FILE_OP_ATTEMPTS; wait_attempts++)
        {
            if (McxCardType(CARD_CHANNEL(g_card_slot)) == MCX_COMMAND_ISSUED)
            {
                break;
            }
            VSync(0);
        }
        if (wait_attempts != CARD_MENU_FILE_OP_ATTEMPTS)
        {
            McxSync(MCX_SYNC_WAIT, &card_command, &card_result);
            if (card_result == McxErrSuccess)
            {
                g_card_step = g_card_step + 1;
                break;
            }
        }
        card_menu_open_status_dialog(3);
        break;

    case CARD_MENU_EXCHANGE_STEP_READ_BEFORE_WRITE:
        g_card_menu_progress_active = 1;
        g_card_menu_progress_bar_active = 1;
        g_card_menu_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (read(g_card_menu_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == CARD_FILE_ERROR)
        {
            close(g_card_menu_file_handle);
            g_card_menu_retry_count = g_card_menu_retry_count - 1;
            if (g_card_menu_retry_count == 0)
            {
                niki_open_secondary_status_dialog(1);
                break;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_EXCHANGE_STEP_POLL_PREWRITE_READ:
        io_result = poll_software_card_events();
        if (io_result == 0)
        {
            g_card_menu_progress_active = 0;
            g_card_step = g_card_step + 1;
            close(g_card_menu_file_handle);
            break;
        }
        if (io_result < 0)
        {
            break;
        }
        if (io_result >= 4)
        {
            break;
        }
        g_card_menu_retry_count = g_card_menu_retry_count - 1;
        if (g_card_menu_retry_count == 0)
        {
            close(g_card_menu_file_handle);
            g_card_menu_progress_bar_active = 0;
            niki_open_secondary_status_dialog(1);
            return phase_result;
        }
        close(g_card_menu_file_handle);
        g_card_step = g_card_step - 1;
        break;

    case CARD_MENU_STEP_INIT_RETRIES:
        g_card_menu_retry_count = CARD_MENU_SAVE_RETRIES;
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_EXCHANGE_STEP_WRITE_SAVE:
        if (g_card_menu_has_free_space == 0)
        {
            /* A full card needs the old save's blocks before writing its replacement. */
            CARD_ERASE_WITH_RETRIES(wait_attempts, g_card_selected_save_path);
        }
        strcat(path.text, g_lom_save_dummy_filename);
        _card_wait(g_card_slot);
        g_card_menu_file_handle = open(path.text, FCREAT | CARD_FILE_BLOCKS(2));
        if (g_card_menu_file_handle == CARD_FILE_ERROR)
        {
            close(g_card_menu_file_handle);
            CARD_ERASE_WITH_RETRIES(wait_attempts, path.text);
            g_card_menu_retry_count = g_card_menu_retry_count - 1;
            if (g_card_menu_retry_count == 0)
            {
                niki_open_secondary_status_dialog(0);
                break;
            }
            break;
        }

        close(g_card_menu_file_handle);
        strcpy(g_card_temp_save_path, path.text);
        _card_wait(g_card_slot);
        g_card_menu_file_handle = open(g_card_temp_save_path, FASYNC | FWRITE);
        clear_software_card_events();
        g_card_menu_progress_bar_active = 1;
        g_card_menu_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        if (write(g_card_menu_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == CARD_FILE_ERROR)
        {
            close(g_card_menu_file_handle);
            CARD_ERASE_WITH_RETRIES(wait_attempts, g_card_temp_save_path);
            g_card_menu_retry_count = g_card_menu_retry_count - 1;
            if (g_card_menu_retry_count == 0)
            {
                niki_open_secondary_status_dialog(0);
                return phase_result;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case CARD_MENU_EXCHANGE_STEP_POLL_SAVE_WRITE:
        io_result = poll_software_card_events();
        switch (io_result)
        {
        case 0:
            if (g_card_menu_has_free_space != 0)
            {
                /* The replacement has finished writing; the old save can now be removed. */
                _card_wait(g_card_slot);
                CARD_ERASE_WITH_RETRIES(wait_attempts, g_card_selected_save_path);
            }
            _card_wait(g_card_slot);
            for (wait_attempts = 0; wait_attempts < CARD_MENU_FILE_OP_ATTEMPTS; wait_attempts++)
            {
                if (rename(g_card_temp_save_path, g_card_selected_save_path) != 0)
                {
                    break;
                }
            }
            g_card_menu_write_in_progress = 0;
            g_card_step = g_card_step + 1;
            close(g_card_menu_file_handle);
            break;
        case 1:
        case 2:
        case 3:
            g_card_menu_retry_count = g_card_menu_retry_count - 1;
            if (g_card_menu_retry_count != 0)
            {
                close(g_card_menu_file_handle);
                g_card_step = g_card_step - 1;
                break;
            }
            g_card_menu_progress_bar_active = 0;
            niki_open_secondary_status_dialog(0);
            CARD_ERASE_WITH_RETRIES(wait_attempts, g_card_temp_save_path);
            break;
        }
        break;
    default:
        return phase_result;
    }

    return phase_result;
}

#include "../../common/card_events/restart_card_sequence.inc.c"
#include "../../common/card_events/poll_and_retry_card_info.inc.c"

#include "../../common/card_menu/card_menu_init_card_events.inc.c"

#include "../../common/card_events/shutdown_card_events.inc.c"

#include "../../common/card_menu/card_menu_begin_entry_scan.inc.c"

/**
 * @brief Read the next directory entry, or rank the completed directory.
 * @param page Memory-card slot being scanned.
 * @return One if another directory entry was read, otherwise zero.
 */
s32 niki_scan_next_entry(s32 page)
{
    s32 selected;

    if (nextfile(&g_card_entries[page][g_card_entry_state]) != 0)
    {
        field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
        g_card_entry_state += 1;
        return 1;
    }

    field_reset_input_repeat();
    if ((g_card_menu_mode == 0) && (card_has_known_entry_type() == 0))
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        g_card_menu_has_free_space = 0;
        if (card_entry_blocks_reach_limit())
        {
            selected = card_rank_entries();
            if (card_has_known_entry_type() == 0)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
                g_card_entry_value_limit = 0;
            }
            else
            {
                g_card_menu_selected_row = selected;
                card_menu_scroll_to_selection();
            }
        }
        else
        {
            g_card_menu_has_free_space = 1;
            selected = card_rank_entries();
            if (card_has_known_entry_type() == 0)
            {
                g_card_menu_selected_row = 0;
                card_menu_scroll_to_selection();
                g_card_entry_value_limit = 0;
            }
            else
            {
                g_card_menu_selected_row = selected;
                card_menu_scroll_to_selection();
            }
        }
    }
    return 0;
}

#include "../../common/card_menu/card_menu_commit_selected_entry.inc.c"

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

#include "../../common/card_directory/card_sort_entries_by_type.inc.c"

#include "../../common/glyph_cache/draw_signed_decimal.inc.c"
#include "../../common/glyph_cache/draw_hex_byte.inc.c"
#include "../../common/glyph_cache/draw_cached_text.inc.c"
#include "../../common/glyph_cache/render_cached_glyph.inc.c"
#include "../../common/glyph_cache/emit_glyph_sprite.inc.c"
#include "../../common/glyph_cache/begin_glyph_cache_frame.inc.c"
#include "../../common/glyph_cache/evict_unused_glyphs.inc.c"
#include "../../common/glyph_cache/reset_glyph_cache.inc.c"

/*
 * The extended table linker symbol is biased backwards by 0x19 pages.  This
 * lets the original code index it directly with the encoded lead byte
 * (0x19..0x1F) instead of subtracting GLYPH_CHART_FIRST_LEAD first.
 */

#include "../../common/glyph_cache/expand_text_glyph_codes.inc.c"
