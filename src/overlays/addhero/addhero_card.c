#include "overlays/field/field_text.h"
#include "internal/addhero_internal.h"
#include <fcntl.h>

extern s32 g_addhero_retry_count;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_file_handle;
extern char g_addhero_target_file_path[];

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
 * @brief Run the current step of the card load/save sequence.
 * @return CARD_MENU_SEQUENCE_* result for the caller.
 * @note g_card_step walks one of the g_addhero_loadseq_* byte tables;
 *       opcodes with no case are no-ops.
 */
s32 addhero_advance_load_sequence(void)
{
    CardSequencePath card_path;
    long card_command;
    long card_result;
    s32 result;
    s32 attempts;
    s32 poll_status;

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    result = CARD_MENU_SEQUENCE_WAIT;
    card_path.device.characters.slot += g_card_slot;

    if (g_card_step != NULL)
    {
        switch (*g_card_step)
        {
        case CARD_MENU_STEP_CARD_INFO:
            result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_info(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_CARD_INFO:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                result = CARD_MENU_SEQUENCE_NO_CARD;
                g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                g_card_step++;
                break;
            case CARD_EVENT_NEW_CARD:
                card_reset_entry_ranks();
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_card_steps_initial_scan;
                break;
            }
            break;

        case CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS:
            clear_software_card_events();
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_HARDWARE_EVENTS:
            do
            {
                poll_status = poll_hardware_card_events();
            } while (poll_status == CARD_EVENT_NONE);
            switch (poll_status)
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                result = CARD_MENU_SEQUENCE_NO_CARD;
                g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                break;
            }
            break;

        case CARD_MENU_STEP_CLEAR_HARDWARE_EVENTS:
            clear_hardware_card_events();
            g_card_step++;
            break;

        case CARD_MENU_STEP_SCAN_ENTRIES:
            card_erase_placeholder_files();
            g_card_menu_entry_scan_active = 1;
            if (card_menu_begin_entry_scan(g_card_slot) == 0)
            {
                result = CARD_MENU_SEQUENCE_FINISHED;
                g_card_step = NULL;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                g_card_menu_entry_scan_active = 0;
                break;
            }
            g_card_step++;
            for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
            {
                if (addhero_scan_next_entry(g_card_slot) == 0)
                {
                    if (g_card_menu_mode != 0)
                    {
                        g_card_menu_selected_row = 0;
                    }
                    g_card_menu_entry_scan_active = 0;
                    if (g_card_entry_state != CARD_MENU_ENTRY_STATE_NO_GAME_DATA && g_card_entry_state != CARD_MENU_ENTRY_STATE_CARD_FULL)
                    {
                        card_menu_commit_selected_entry();
                    }
                    break;
                }
            }
            break;

        case CARD_MENU_STEP_CLEAR_CARD:
            result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_clear(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case CARD_MENU_STEP_LOAD_CARD:
            result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_load(CARD_CHANNEL(g_card_slot));
            g_addhero_primary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
            g_addhero_secondary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
            g_card_step++;
            break;

        case CARD_MENU_STEP_DONE:
            result = CARD_MENU_SEQUENCE_FINISHED;
            g_card_menu_write_in_progress = 0;
            break;

        case CARD_MENU_EXCHANGE_STEP_ERASE_ENTRY:
            strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
            for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
            {
                if (erase(card_path.text) != 0)
                {
                    break;
                }
            }
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_CARD_LOAD:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                g_addhero_secondary_poll_countdown--;
                if (g_addhero_secondary_poll_countdown != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    _card_load(CARD_CHANNEL(g_card_slot));
                }
                else
                {
                    result = CARD_MENU_SEQUENCE_NO_CARD;
                    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                }
                break;
            case CARD_EVENT_NEW_CARD:
                g_addhero_primary_poll_countdown--;
                if (g_addhero_primary_poll_countdown != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    _card_load(CARD_CHANNEL(g_card_slot));
                }
                else
                {
                    result = CARD_MENU_SEQUENCE_UNFORMATTED;
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_SAVE_DATA;
                    g_card_step = g_card_steps_idle;
                }
                break;
            }
            break;

        case CARD_MENU_STEP_WAIT_HARDWARE_EVENTS:
            do
            {
                poll_status = poll_hardware_card_events();
            } while (poll_status == CARD_EVENT_NONE);
            g_card_step++;
            break;

        case CARD_MENU_STEP_READ_ENTRY:
            g_card_menu_io_busy = 1;
            g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            if (g_addhero_file_handle == -1)
            {
                break;
            }
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_entry_file,
                     g_card_menu_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : CARD_MENU_ENTRY_TITLE_READ_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                break;
            }
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_ENTRY_READ:
            if (g_card_menu_io_busy != 0)
            {
                poll_status = poll_software_card_events();
                if (poll_status == CARD_EVENT_COMPLETE)
                {
                    g_card_menu_io_busy = 0;
                    g_card_menu_selection_status = CARD_MENU_SELECTION_ENTRY_READ;
                    close(g_addhero_file_handle);
                }
                else if (poll_status != CARD_EVENT_NONE)
                {
                    close(g_addhero_file_handle);
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                    g_card_step = g_card_steps_initial_scan;
                }
            }
            else
            {
                g_card_step++;
            }
            break;

        case CARD_MENU_STEP_READ_SAVE:
            g_card_menu_progress_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    card_menu_open_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_SAVE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_menu_progress_active = 0;
                g_card_step++;
                close(g_addhero_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_card_menu_progress_bar_active = 0;
                    card_menu_open_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                    return result;
                }
                /* Rewind to CARD_MENU_STEP_READ_SAVE and try again. */
                g_card_step--;
                break;
            }
            break;

        case CARD_MENU_STEP_CHECK_POCKETSTATION:
            for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
            {
                if (McxCardType(CARD_CHANNEL(g_card_slot)) == MCX_COMMAND_ISSUED)
                {
                    break;
                }
                VSync(0);
            }
            if (attempts != CARD_MENU_FILE_OP_ATTEMPTS)
            {
                McxSync(MCX_SYNC_WAIT, &card_command, &card_result);
                if (card_result == McxErrSuccess)
                {
                    g_card_step++;
                    break;
                }
            }
            card_menu_open_status_dialog(CARD_MENU_DIALOG_NOT_POCKETSTATION);
            break;

        case CARD_MENU_STEP_INIT_RETRIES:
            g_addhero_retry_count = CARD_MENU_SAVE_RETRIES;
            g_card_step++;
            break;

        case CARD_MENU_EXCHANGE_STEP_READ_BEFORE_WRITE:
            g_card_menu_progress_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case CARD_MENU_EXCHANGE_STEP_POLL_PREWRITE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_menu_progress_active = 0;
                g_card_step++;
                close(g_addhero_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_card_menu_progress_bar_active = 0;
                    addhero_open_exit_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                    return result;
                }
                /* Rewind to CARD_MENU_EXCHANGE_STEP_READ_BEFORE_WRITE and try again. */
                close(g_addhero_file_handle);
                g_card_step--;
                break;
            }
            break;

        case CARD_MENU_EXCHANGE_STEP_WRITE_SAVE:
            if (g_addhero_has_free_entry_space == 0)
            {
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_card_selected_save_path) != 0)
                    {
                        break;
                    }
                }
            }
            strcat(card_path.text, g_lom_save_dummy_filename);
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(card_path.text, FCREAT | CARD_FILE_BLOCKS(SAVE_FILE_BYTES / CARD_BLOCK_BYTES));
            if (g_addhero_file_handle == -1)
            {
                close(-1);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(card_path.text) != 0)
                    {
                        break;
                    }
                }
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return result;
                }
                break;
            }
            close(g_addhero_file_handle);
            strcpy(g_addhero_target_file_path, card_path.text);
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_addhero_target_file_path, FASYNC | FWRITE);
            clear_software_card_events();
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            if (write(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_addhero_target_file_path) != 0)
                    {
                        break;
                    }
                }
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case CARD_MENU_EXCHANGE_STEP_POLL_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                if (g_addhero_has_free_entry_space != 0)
                {
                    _card_wait(g_card_slot);
                    for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_card_selected_save_path) != 0)
                        {
                            break;
                        }
                    }
                }
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_addhero_target_file_path, g_card_selected_save_path) != 0)
                    {
                        break;
                    }
                }
                g_card_menu_write_in_progress = 0;
                g_card_step++;
                close(g_addhero_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_card_menu_progress_bar_active = 0;
                    addhero_open_exit_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return result;
                }
                /* Rewind to CARD_MENU_EXCHANGE_STEP_WRITE_SAVE and try again. */
                close(g_addhero_file_handle);
                g_card_step--;
                break;
            }
            break;
        }
    }
    return result;
}

#include "../../common/card_events/restart_card_sequence.inc.c"
#include "../../common/card_events/poll_and_retry_card_info.inc.c"

#include "../../common/card_menu/card_menu_init_card_events.inc.c"

#include "../../common/card_events/shutdown_card_events.inc.c"

#include "../../common/card_menu/card_menu_begin_entry_scan.inc.c"

/**
 * @brief Read the next directory entry, or finish the scan and select a save.
 * @param page Memory-card slot being scanned.
 * @return 1 if another entry was read, 0 when the scan ends.
 */
s32 addhero_scan_next_entry(s32 page)
{
    s32 selected_entry;

    if (nextfile(&g_card_entries[page][g_card_entry_state]) != 0)
    {
        field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
        g_card_entry_state += 1;
        return 1;
    }

    field_reset_input_repeat();
    if (g_card_menu_mode == 0 && card_has_known_entry_type() == 0)
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        g_addhero_has_free_entry_space = 0;
        if (card_entry_blocks_reach_limit())
        {
            selected_entry = card_rank_entries();
            if (card_has_known_entry_type() == 0)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
                g_card_entry_value_limit = 0;
            }
            else
            {
                if (g_card_menu_mode != 0)
                {
                    g_card_menu_selected_row = 0;
                }
                g_card_menu_selected_row = selected_entry;
                card_menu_scroll_to_selection();
            }
        }
        else
        {
            g_addhero_has_free_entry_space = 1;
            selected_entry = card_rank_entries();
            if (card_has_known_entry_type() == 0)
            {
                g_card_menu_selected_row = 0;
                card_menu_scroll_to_selection();
                g_card_entry_value_limit = 0;
            }
            else
            {
                if (g_card_menu_mode != 0)
                {
                    g_card_menu_selected_row = 0;
                }
                g_card_menu_selected_row = selected_entry;
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

/**
 * @brief Group Mana saves by suffix, then PocketStation saves, new saves and other files.
 */
void card_sort_entries_by_type(void)
{
    struct DIRENTRY sorted[CARD_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < CARD_MENU_ENTRY_GROUP_COUNT; suffix++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < CARD_MENU_ENTRY_GROUP_COUNT; suffix++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) != 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        bcopy((u8*)&sorted[entry_index], (u8*)&g_card_entries[g_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}
