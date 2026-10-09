#include "internal/niki_internal.h"
#include "overlays/menu/menu.h"
#include <fcntl.h>

s32 niki_has_known_entry_type(void);

/** @brief Remove placeholder save files before starting the card operation. */
static inline void niki_erase_placeholder_paths(void)
{
    CardFilePath p;

    memcpy(&p, &g_niki_file_template, CARD_DEVICE_BYTES);
    p.device.characters.slot += (u8)g_card_slot;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &g_niki_file_template, CARD_DEVICE_BYTES);
    p.device.characters.slot += (u8)g_card_slot;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

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
    s32 rank_index;
    s32 rank_value;
    s32 command;

    memcpy(&path, &g_niki_file_template, CARD_DEVICE_BYTES);
    phase_result = 1;
    path.device.characters.slot += (u8)g_card_slot;

    if (g_card_step == NULL)
    {
        return phase_result;
    }

    command = *g_card_step;
    switch (command)
    {
    case NIKI_COMMAND_REQUEST_CARD_INFO:
        phase_result = 3;
        _card_wait(g_card_slot);
        _card_info(g_card_slot * 0x10);
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_CARD_INFO:
        poll_result = poll_software_card_events();
        switch (poll_result)
        {
        case 1:
        case 2:
            phase_result = 4;
            g_niki_selection_status = 0;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
            g_card_step = g_card_step + 1;
            break;
        case 3:
            g_niki_rank_count = 0x28;
            rank_value = -1;
            for (rank_index = 14; rank_index >= 0; rank_index--)
            {
                g_niki_entry_ranks[rank_index] = rank_value;
            }
            g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
            g_card_step = g_niki_card_setup_sequence;
            break;
        case 0:
            g_card_step = g_card_step + 1;
            break;
        default:
            return phase_result;
        }
        break;

    case NIKI_COMMAND_RELEASE_PRIMARY:
        clear_software_card_events();
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_WAIT_SECONDARY:
        do
        {
            poll_result = poll_hardware_card_events();
        } while (poll_result == -1);
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
        phase_result = 4;
        g_niki_selection_status = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
        break;

    case NIKI_COMMAND_RELEASE_SECONDARY:
        clear_hardware_card_events();
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_SCAN_DIRECTORY:
        niki_erase_placeholder_paths();
        g_niki_entry_scan_active = 1;
        if (niki_begin_entry_scan(g_card_slot) == 0)
        {
            phase_result = 2;
            g_card_step = NULL;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
            g_niki_entry_scan_active = 0;
            break;
        }
        wait_attempts = 0;
        g_card_step = g_card_step + 1;
        do
        {
            if (niki_scan_next_entry(g_card_slot) == 0)
            {
                if (g_niki_mode != 0)
                {
                    g_niki_selected_row = 0;
                }
                g_niki_entry_scan_active = 0;
                if (g_card_entry_state == CARD_MENU_ENTRY_STATE_NO_GAME_DATA)
                {
                    return phase_result;
                }
                if (g_card_entry_state == CARD_MENU_ENTRY_STATE_CARD_FULL)
                {
                    break;
                }
                niki_commit_selected_entry();
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 20);
        break;

    case NIKI_COMMAND_REQUEST_CARD_CLEAR:
        phase_result = 3;
        _card_wait(g_card_slot);
        func_800172AC(g_card_slot * 0x10);
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_REQUEST_CARD_LOAD:
        phase_result = 3;
        _card_wait(g_card_slot);
        func_8001725C(g_card_slot * 0x10);
        g_niki_primary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
        g_niki_secondary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_STOP:
        phase_result = 2;
        g_niki_progress_active = 0;
        break;

    case NIKI_COMMAND_ERASE_SELECTED_FILE:
        func_80016F9C(&path, g_card_entries[g_card_slot][g_niki_selected_row].name);
        wait_attempts = 0;
        _card_wait(g_card_slot);
        do
        {
            poll_result = func_8001686C(&path);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        } while (wait_attempts < 20);
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_CARD_READY:
        poll_result = poll_software_card_events();
        switch (poll_result)
        {
        case 1:
        case 2:
            g_niki_secondary_poll_countdown = g_niki_secondary_poll_countdown - 1;
            if (g_niki_secondary_poll_countdown == 0)
            {
                phase_result = 4;
                g_niki_selection_status = 0;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                break;
            }
            _card_wait(g_card_slot);
            func_800172AC(g_card_slot * 0x10);
            _card_wait(g_card_slot);
            func_8001725C(g_card_slot * 0x10);
            break;
        case 3:
            g_niki_primary_poll_countdown = g_niki_primary_poll_countdown - 1;
            if (g_niki_primary_poll_countdown != 0)
            {
                _card_wait(g_card_slot);
                func_800172AC(g_card_slot * 0x10);
                _card_wait(g_card_slot);
                func_8001725C(g_card_slot * 0x10);
                break;
            }
            phase_result = 5;
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_SAVE_DATA;
            g_card_step = g_card_steps_idle;
            break;
        case 0:
            g_card_step = g_card_step + 1;
            break;
        }
        break;

    case NIKI_COMMAND_WAIT_SECONDARY_COMPLETE:
        do
        {
            poll_result = poll_hardware_card_events();
        } while (poll_result == -1);
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_READ_ENTRY_PREVIEW:
        g_niki_io_busy = 1;
        g_niki_selection_status = 0;
        _card_wait(g_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, FASYNC | FREAD);
        if (g_niki_file_handle == -1)
        {
            break;
        }
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (func_8001681C(g_niki_file_handle, &g_niki_entry_file, g_niki_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : 0x80) == -1)
        {
            func_8001683C(g_niki_file_handle);
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_ENTRY_PREVIEW:
        poll_result = poll_software_card_events();
        if (poll_result == 0)
        {
            g_niki_io_busy = 0;
            g_niki_selection_status = 1;
            g_card_step = g_card_step + 1;
            func_8001683C(g_niki_file_handle);
            break;
        }
        if (poll_result == -1)
        {
            break;
        }
        g_niki_io_busy = 0;
        func_8001683C(g_niki_file_handle);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_card_step = g_niki_card_setup_sequence;
        break;

    case NIKI_COMMAND_READ_SAVE:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, FASYNC | FREAD);
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == -1)
        {
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
                niki_open_status_dialog(1);
                break;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_SAVE_READ:
        io_result = poll_software_card_events();
        if (io_result == 0)
        {
            g_niki_confirm_latch = 0;
            g_card_step = g_card_step + 1;
            func_8001683C(g_niki_file_handle);
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
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
            g_niki_progress_bar_active = 0;
            niki_open_status_dialog(1);
            return phase_result;
        }
        g_card_step = g_card_step - 1;
        break;

    case NIKI_COMMAND_CHECK_POCKETSTATION:
        wait_attempts = 0;
        do
        {
            if (McxCardType(g_card_slot * 0x10) == MCX_COMMAND_ISSUED)
            {
                break;
            }
            VSync(0);
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 20);
        if (wait_attempts != 20)
        {
            McxSync(MCX_SYNC_WAIT, &card_command, &card_result);
            if (card_result == McxErrSuccess)
            {
                g_card_step = g_card_step + 1;
                break;
            }
        }
        niki_open_status_dialog(3);
        break;

    case NIKI_COMMAND_READ_SAVED_COPY:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, FASYNC | FREAD);
        clear_software_card_events();
        _card_wait(g_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == -1)
        {
            func_8001683C(g_niki_file_handle);
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
                niki_open_secondary_status_dialog(1);
                break;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_SAVED_COPY:
        io_result = poll_software_card_events();
        if (io_result == 0)
        {
            g_niki_confirm_latch = 0;
            g_card_step = g_card_step + 1;
            func_8001683C(g_niki_file_handle);
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
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
            func_8001683C(g_niki_file_handle);
            g_niki_progress_bar_active = 0;
            niki_open_secondary_status_dialog(1);
            return phase_result;
        }
        func_8001683C(g_niki_file_handle);
        g_card_step = g_card_step - 1;
        break;

    case NIKI_COMMAND_RESET_RETRIES:
        g_niki_retry_count = CARD_MENU_SAVE_RETRIES;
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_WRITE_SAVE:
        if (g_niki_preserve_old_save == 0)
        {
            /* A full card needs the old save's blocks before writing its replacement. */
            wait_attempts = 0;
            do
            {
                if (func_8001686C(g_niki_selected_save_path) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 20);
        }
        func_80016F9C(&path, D_800ECF9C);
        _card_wait(g_card_slot);
        g_niki_file_handle = func_8001680C(&path, FCREAT | CARD_FILE_BLOCKS(2));
        if (g_niki_file_handle == -1)
        {
            func_8001683C(-1);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&path) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 20);
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
                niki_open_secondary_status_dialog(0);
                break;
            }
            break;
        }

        func_8001683C(g_niki_file_handle);
        func_800170BC(g_niki_temporary_save_path, &path);
        _card_wait(g_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_temporary_save_path, FASYNC | FWRITE);
        clear_software_card_events();
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = VSync(-1);
        _card_wait(g_card_slot);
        if (func_8001682C(g_niki_file_handle, g_niki_save_blob.bytes, SAVE_FILE_BYTES) == -1)
        {
            func_8001683C(g_niki_file_handle);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(g_niki_temporary_save_path) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 20);
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
                niki_open_secondary_status_dialog(0);
                return phase_result;
            }
            break;
        }
        g_card_step = g_card_step + 1;
        break;

    case NIKI_COMMAND_POLL_SAVE_WRITE:
        io_result = poll_software_card_events();
        switch (io_result)
        {
        case 0:
            if (g_niki_preserve_old_save != 0)
            {
                /* The replacement has finished writing; the old save can now be removed. */
                _card_wait(g_card_slot);
                wait_attempts = 0;
                do
                {
                    if (func_8001686C(g_niki_selected_save_path) != 0)
                    {
                        break;
                    }
                    wait_attempts = wait_attempts + 1;
                } while (wait_attempts < 20);
            }
            _card_wait(g_card_slot);
            wait_attempts = 0;
            do
            {
                if (func_8001685C(g_niki_temporary_save_path, g_niki_selected_save_path) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 20);
            g_niki_progress_active = 0;
            g_card_step = g_card_step + 1;
            func_8001683C(g_niki_file_handle);
            break;
        case 1:
        case 2:
        case 3:
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count != 0)
            {
                func_8001683C(g_niki_file_handle);
                g_card_step = g_card_step - 1;
                break;
            }
            g_niki_progress_bar_active = 0;
            niki_open_secondary_status_dialog(0);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(g_niki_temporary_save_path) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 20);
            break;
        }
        break;
    default:
        return phase_result;
    }

    return phase_result;
}

/** @brief Wildcard matching all files on memory-card slot zero. */
const CardPathTemplate g_niki_entry_header_template = {"bu00:*"};

#include "../../common/card_events/restart_card_sequence.inc.c"
#include "../../common/card_events/poll_and_retry_card_info.inc.c"

/**
 * @brief Open and enable software and hardware memory-card events for polling.
 */
void niki_init_card_events(void)
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
    g_niki_progress_bar_active = 0;
    g_niki_entry_scan_active = 0;
}

#include "../../common/card_events/shutdown_card_events.inc.c"

/**
 * @brief Reset the browser and read the selected card's first directory entry.
 * @param card_slot Memory-card slot to scan.
 * @return One if the first entry was read, otherwise zero.
 */
s32 niki_begin_entry_scan(s32 card_slot)
{
    CardSearchPattern pattern;

    memcpy(&pattern, &g_niki_entry_header_template, CARD_SEARCH_PATTERN_BYTES);
    g_niki_selected_row = 0;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_card_entry_state = 0;
    pattern.device.characters.slot += card_slot;
    if (func_80016BCC(&pattern, g_card_entries[card_slot]) != 0)
    {
        func_800B0170(&g_card_entries[card_slot][g_card_entry_state]);
        g_card_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Read the next directory entry, or rank the completed directory.
 * @param page Memory-card slot being scanned.
 * @return One if another directory entry was read, otherwise zero.
 */
s32 niki_scan_next_entry(s32 page)
{
    s32 used_blocks;
    s32 entry_index;
    s32 selected;
    s32 card_full;

    if (func_8001684C(&g_card_entries[page][g_card_entry_state]) != 0)
    {
        func_800B0170(&g_card_entries[page][g_card_entry_state]);
        g_card_entry_state += 1;
        return 1;
    }

    field_reset_input_repeat();
    if ((g_niki_mode == 0) && (niki_has_known_entry_type() == 0))
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        used_blocks = 0;
        g_niki_preserve_old_save = 0;
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            used_blocks += g_card_entries[g_card_slot][entry_index].size / CARD_BLOCK_BYTES;
        }
        card_full = used_blocks >= 0xE;
        if (card_full != 0)
        {
            selected = niki_rank_entries();
            if (niki_has_known_entry_type() == 0)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
                g_niki_entry_value_limit = 0;
            }
            else
            {
                g_niki_selected_row = selected;
                niki_scroll_to_selection();
            }
        }
        else
        {
            g_niki_preserve_old_save = 1;
            selected = niki_rank_entries();
            if (niki_has_known_entry_type() == 0)
            {
                g_niki_selected_row = 0;
                niki_scroll_to_selection();
                g_niki_entry_value_limit = 0;
            }
            else
            {
                g_niki_selected_row = selected;
                niki_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Prepare the selected save-file path and begin its load sequence.
 */
void niki_commit_selected_entry(void)
{
    CardEntryPath path;
    u8* path_bytes;

    if (g_card_entry_state == 0)
    {
        g_niki_selection_status = 3;
        return;
    }
    {
        if (strncmp(&D_800ECFC4[0], g_card_entries[g_card_slot][g_niki_selected_row].name, 8) == 0)
        {
            g_niki_selection_status = 2;
            return;
        }
    }
    memcpy(&path, &g_niki_file_template, CARD_DEVICE_BYTES);
    path_bytes = (u8*)&path;
    {
        func_80016F9C(path_bytes, g_card_entries[g_card_slot][g_niki_selected_row].name);
    }
    {
        s32 slot;
        s32 value;
        value = path.device.characters.slot;
        slot = (u8)g_card_slot;
        g_niki_selection_status = 0;
        value += slot;
        path.device.characters.slot = value;
        func_800170BC(&g_niki_selected_save_path[0], path_bytes, slot);
    }
    g_card_step = &g_niki_preview_sequence[0];
    {
        if (strncmp(&g_lom_save_filename_prefix[0], g_card_entries[g_card_slot][g_niki_selected_row].name, 0xC) == 0)
        {
            g_niki_selected_entry_extended = 1;
        }
        else
        {
            g_niki_selected_entry_extended = 0;
        }
    }
    g_niki_io_busy = 1;
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

/**
 * @brief Group recognized save-file types by suffix, then append other entries.
 * @note Preserves directory order within each type and suffix group.
 */
void niki_sort_entries_by_type(void)
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
                strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_card_entries[g_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < CARD_MENU_ENTRY_GROUP_COUNT; suffix++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == suffix && strncmp(D_800ECF8C, g_card_entries[g_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_card_entries[g_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(D_800ECFC4, g_card_entries[g_card_slot][entry_index].name, 8) == 0)
        {
            func_80016E7C(&g_card_entries[g_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 12) != 0 &&
            strncmp(D_800ECF8C, g_card_entries[g_card_slot][entry_index].name, 12) != 0 &&
            strncmp(D_800ECFC4, g_card_entries[g_card_slot][entry_index].name, 8) != 0)
        {
            func_80016E7C(&g_card_entries[g_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        func_80016E7C(&sorted[entry_index], &g_card_entries[g_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}

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
 * (0x19..0x1F) instead of subtracting NIKI_TEXT_EXTENDED_LEAD_FIRST first.
 */

#include "../../common/glyph_cache/expand_text_glyph_codes.inc.c"
