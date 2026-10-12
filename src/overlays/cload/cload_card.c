#include "internal/cload_internal.h"
#include <memory.h>
#include <strings.h>
#include <fcntl.h>
#include <libetc.h>
#include "main/controller.h"
#include "overlays/field/field_input.h"

void* bcopy(const unsigned char* src, unsigned char* dst, int count);

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"
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
 * @brief Run the current memory-card load step and advance g_card_step.
 * @return CardMenuSequenceResult phase code for card_menu_update_card_sequence.
 * @note g_card_step walks one of the g_card_steps_* byte tables; each
 *       CardMenuStep opcode issues or polls a card command, reads the selected
 *       save, or scans the card directory, and updates g_card_entry_state /
 *       g_card_menu_selection_status. Opcodes with no case are no-ops.
 */
s32 cload_advance_load_sequence(void)
{
    CardSequencePath card_path;
    long status0;
    long status1;
    s32 phase_result;
    s32 scan_attempts;
    s32 wait_attempts;
    s32 poll_result;

    /* Builds the "bu00:" slot path like the erase helper; never used afterwards. */
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    phase_result = CARD_MENU_SEQUENCE_WAIT;
    card_path.device.characters.slot += g_card_slot;

    if (g_card_step != NULL)
    {
        switch (*g_card_step)
        {
        case CARD_MENU_STEP_CARD_INFO:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
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
                phase_result = CARD_MENU_SEQUENCE_NO_CARD;
                g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                g_card_step++;
                card_menu_deactivate_primary_element();
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
                poll_result = poll_hardware_card_events();
            } while (poll_result == -1);
            if (poll_result == CARD_EVENT_COMPLETE)
            {
                g_card_step++;
                break;
            }
            if (poll_result < 0)
            {
                return phase_result;
            }
            if (poll_result > CARD_EVENT_NEW_CARD)
            {
                return phase_result;
            }
            phase_result = CARD_MENU_SEQUENCE_NO_CARD;
            g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
            break;

        case CARD_MENU_STEP_CLEAR_HARDWARE_EVENTS:
            clear_hardware_card_events();
            g_card_step++;
            break;

        case CARD_MENU_STEP_SCAN_ENTRIES:
            card_erase_placeholder_files();
            g_card_menu_entry_scan_active = 1;
            if (cload_begin_entry_scan(g_card_slot) == 0)
            {
                phase_result = CARD_MENU_SEQUENCE_FINISHED;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                g_card_step = NULL;
                g_card_menu_entry_scan_active = 0;
                break;
            }
            scan_attempts = 0;
            g_card_step++;
            do
            {
                if (cload_scan_next_entry(g_card_slot) == 0)
                {
                    g_card_menu_entry_scan_active = 0;
                    if (g_card_entry_state == CARD_MENU_ENTRY_STATE_NO_GAME_DATA)
                    {
                        return phase_result;
                    }
                    if (g_card_entry_state == CARD_MENU_ENTRY_STATE_CARD_FULL)
                    {
                        return phase_result;
                    }
                    cload_commit_selected_entry();
                    break;
                }
                scan_attempts++;
            } while (scan_attempts < CARD_MENU_FILE_OP_ATTEMPTS);
            break;

        case CARD_MENU_STEP_CLEAR_CARD:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_clear(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case CARD_MENU_STEP_LOAD_CARD:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_load(CARD_CHANNEL(g_card_slot));
            g_card_menu_new_card_retries = CARD_MENU_CARD_LOAD_RETRIES;
            g_card_menu_load_error_retries = CARD_MENU_CARD_LOAD_RETRIES;
            g_card_step++;
            break;

        case CARD_MENU_STEP_DONE:
            phase_result = CARD_MENU_SEQUENCE_FINISHED;
            g_card_menu_write_in_progress = 0;
            break;

        case CARD_MENU_STEP_POLL_CARD_LOAD:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                g_card_menu_load_error_retries--;
                if (g_card_menu_load_error_retries == 0)
                {
                    phase_result = CARD_MENU_SEQUENCE_NO_CARD;
                    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                }
                else
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    _card_load(CARD_CHANNEL(g_card_slot));
                }
                break;
            case CARD_EVENT_NEW_CARD:
                g_card_menu_new_card_retries--;
                if (g_card_menu_new_card_retries != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    _card_load(CARD_CHANNEL(g_card_slot));
                }
                else
                {
                    phase_result = CARD_MENU_SEQUENCE_UNFORMATTED;
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_SAVE_DATA;
                    g_card_step = g_card_steps_idle;
                }
                break;
            }
            break;

        case CARD_MENU_STEP_WAIT_HARDWARE_EVENTS:
            do
            {
                poll_result = poll_hardware_card_events();
            } while (poll_result == -1);
            g_card_step++;
            break;

        case CARD_MENU_STEP_READ_ENTRY:
            g_card_menu_io_busy = 1;
            g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            if (g_card_menu_file_handle == -1)
            {
                break;
            }
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_card_menu_file_handle, &g_cload_selected_file,
                     g_card_menu_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : CARD_MENU_ENTRY_TITLE_READ_BYTES) != -1)
            {
                g_card_step++;
            }
            else
            {
                close(g_card_menu_file_handle);
            }
            break;

        case CARD_MENU_STEP_POLL_ENTRY_READ:
            poll_result = poll_software_card_events();
            if (poll_result == CARD_EVENT_COMPLETE)
            {
                g_card_menu_io_busy = 0;
                g_card_menu_selection_status = CARD_MENU_SELECTION_ENTRY_READ;
                g_card_step++;
                close(g_card_menu_file_handle);
            }
            else if (poll_result != -1)
            {
                g_card_menu_io_busy = 0;
                close(g_card_menu_file_handle);
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_card_steps_initial_scan;
            }
            break;

        case CARD_MENU_STEP_INIT_RETRIES:
            g_card_menu_retry_count = CARD_MENU_SAVE_RETRIES;
            g_card_step++;
            break;

        case CARD_MENU_STEP_READ_SAVE:
            g_card_menu_progress_active = 1;
            g_card_menu_progress_bar_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_card_menu_file_handle, &g_cload_save_file, sizeof(g_cload_save_file)) != -1)
            {
                g_card_step++;
                break;
            }
            close(g_card_menu_file_handle);
            g_card_menu_retry_count--;
            if (g_card_menu_retry_count == 0)
            {
                cload_open_status_dialog(1);
            }
            break;

        case CARD_MENU_STEP_POLL_SAVE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_menu_progress_active = 0;
                g_card_step++;
                close(g_card_menu_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count != 0)
                {
                    /* Rewind to CARD_MENU_STEP_READ_SAVE and try again. */
                    close(g_card_menu_file_handle);
                    g_card_step--;
                    break;
                }
                close(g_card_menu_file_handle);
                g_card_menu_progress_bar_active = 0;
                cload_open_status_dialog(1);
                break;
            }
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
                McxSync(MCX_SYNC_WAIT, &status0, &status1);
                if (status1 == McxErrSuccess)
                {
                    g_card_step++;
                    break;
                }
            }
            cload_open_status_dialog(CARD_MENU_DIALOG_NOT_POCKETSTATION);
            break;
        }
    }
    return phase_result;
}

/**
 * @brief Reset the cached resource handles and arm the first load step.
 * @note Releases the handles (clear_software_card_events), rewinds the CD channel, and points
 *       g_card_step at the g_card_steps_idle step table.
 */
void cload_restart_load_sequence(void)
{
    clear_software_card_events();
    _card_wait(g_card_slot);
    _card_info(CARD_CHANNEL(g_card_slot));
    g_card_step = g_card_steps_idle;
}

#include "../../common/card_events/poll_and_retry_card_info.inc.c"

/**
 * @brief Allocate and register the eight streaming buffers for this overlay.
 * @note Brackets the eight OpenEvent allocations (handles stored in
 *       g_card_software_event_io_complete..g_card_hardware_event_new_card) with EnterCriticalSection / ExitCriticalSection and resets the
 *       stream bookkeeping (g_card_menu_entry_scan_active, g_card_menu_progress_start_tick, g_card_menu_progress_bar_active).
 */
void cload_init_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_card_software_event_io_complete = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_card_software_event_error = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_card_software_event_timeout = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_card_software_event_new_card = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    g_card_hardware_event_io_complete = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_card_hardware_event_error = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_card_hardware_event_timeout = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_card_hardware_event_new_card = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    EnableEvent(g_card_software_event_io_complete);
    EnableEvent(g_card_software_event_error);
    EnableEvent(g_card_software_event_timeout);
    EnableEvent(g_card_software_event_new_card);
    EnableEvent(g_card_hardware_event_io_complete);
    EnableEvent(g_card_hardware_event_error);
    EnableEvent(g_card_hardware_event_timeout);
    EnableEvent(g_card_hardware_event_new_card);
    ExitCriticalSection();
    g_card_menu_entry_scan_active = 0;
    g_card_menu_progress_start_tick = VSync(-1);
    g_card_menu_progress_bar_active = 0;
}

#include "../../common/card_events/shutdown_card_events.inc.c"

/**
 * @brief Begin streaming the page's first g_card_entries record.
 * @param page Page index (each page is 0x320 bytes in g_card_entries).
 * @return 1 if firstfile accepted the record (count bumped), else 0.
 */
s32 cload_begin_entry_scan(s32 page)
{
    CardSearchPattern search_path;

    strcpy(search_path.text, CARD_SEARCH_PATTERN);
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_card_entry_state = 0;
    search_path.device.characters.slot += page;
    if (firstfile(search_path.text, &g_card_entries[page][0]) != 0)
    {
        g_card_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Try to append the page's next g_card_entries record; if it cannot,
 *        recompute the page's fixed-point total and update the selection state.
 * @param page Page index (each page is 0x320 bytes / 20 records in g_card_entries).
 * @return 1 if nextfile accepted the new record (count bumped), else 0.
 * @note When the directory is complete, card_entry_blocks_reach_limit decides
 *       whether a full card clamps the state (0xFA) or the selection
 *       (g_card_menu_selected_row) is set from card_rank_entries's result.
 */
s32 cload_scan_next_entry(s32 page)
{
    s32 selected;

    if (nextfile(&g_card_entries[page][g_card_entry_state]) != 0)
    {
        g_card_entry_state += 1;
        return 1;
    }
    field_reset_input_repeat();
    if (card_has_known_entry_type() == 0)
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
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
                cload_scroll_to_selection();
            }
        }
        else
        {
            selected = card_rank_entries();
            if (card_has_known_entry_type() == 0)
            {
                g_card_menu_selected_row = 0;
                cload_scroll_to_selection();
                g_card_entry_value_limit = 0;
            }
            else
            {
                g_card_menu_selected_row = selected;
                cload_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Commit the selected g_card_entries record and arm the next step.
 * @note Rejects the new-save placeholder entry, builds "buX0:<name>" for the
 *       selected entry, copies it to g_card_selected_save_path, and flags
 *       whether the entry is an extended (g_lom_save_filename_prefix) save.
 */
void cload_commit_selected_entry(void)
{
    CardEntryPath card_path;

    if (g_card_entry_state == 0)
    {
        g_card_menu_selection_status = CARD_MENU_SELECTION_EMPTY_CARD;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
    {
        g_card_menu_selection_status = CARD_MENU_SELECTION_NEW_SAVE;
        return;
    }
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
    card_path.device.characters.slot += (u8)g_card_slot;
    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
    strcpy(g_card_selected_save_path, card_path.text);
    g_card_step = &g_card_steps_read_selected_header[0];
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_card_menu_selected_entry_extended = 1;
        return;
    }
    g_card_menu_selected_entry_extended = 0;
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

#include "../../common/card_directory/card_sort_entries_by_type.inc.c"
