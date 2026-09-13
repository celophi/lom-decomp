#include "niki_internal.h"

/** @brief Remove placeholder save files before starting the card operation. */
static inline void niki_erase_placeholder_paths(void)
{
    NikiPlaceholderPath p;

    memcpy(&p, &g_niki_file_template, 6);
    p.device.characters.slot += (u8)g_niki_card_slot;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &g_niki_file_template, 6);
    p.device.characters.slot += (u8)g_niki_card_slot;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

/**
 * @brief Execute the current memory-card load/save command and advance its sequence.
 * @return Command result reported by the current sequence step.
 */
s32 niki_advance_load_sequence(void)
{
    NikiSequencePath path;
    s32 card_command;
    s32 card_result;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 io_result;
    s32 rank_index;
    s32 rank_value;
    s32 command;

    memcpy(&path, &g_niki_file_template, 6);
    phase_result = 1;
    path.device.characters.slot += (u8)g_niki_card_slot;

    if (g_niki_load_step == NULL)
    {
        return phase_result;
    }

    command = *g_niki_load_step;
    switch (command)
    {

    case 1:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_8001724C(g_niki_card_slot * 0x10);
        g_niki_load_step = g_niki_load_step + 1;
        break;

    case 2:
        poll_result = niki_poll_primary_handle_group();
        if (poll_result >= 3)
        {
            goto check_card_info_change;
        }
        if (poll_result > 0)
        {
            goto card_info_error;
        }
        if (poll_result == 0)
        {
            goto block_increment;
        }
        return phase_result;
    check_card_info_change:
        if (poll_result == 3)
        {
            goto card_info_changed;
        }
        return phase_result;
    card_info_error:
        phase_result = 4;
        g_niki_selection_status = 0;
        g_niki_entry_state = 0xFD;
        g_niki_load_step = g_niki_load_step + 1;
        break;
    card_info_changed:
        g_niki_rank_count = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            g_niki_entry_ranks[rank_index] = rank_value;
        }
        goto block_status_ff;

    case 3:
        niki_release_primary_handles();
        goto block_increment;

    case 4:
        do
        {
            poll_result = niki_poll_secondary_handle_group();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            goto block_increment;
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
        goto block_status_fd;

    case 5:
        niki_release_secondary_handles();
        goto block_increment;

    case 6:
        niki_erase_placeholder_paths();
        g_niki_entry_scan_active = 1;
        if (niki_begin_entry_scan(g_niki_card_slot) == 0)
        {
            phase_result = 2;
            g_niki_load_step = NULL;
            g_niki_entry_state = 0xF8;
            g_niki_entry_scan_active = 0;
            break;
        }
        wait_attempts = 0;
        g_niki_load_step = g_niki_load_step + 1;
        do
        {
            if (niki_scan_next_entry(g_niki_card_slot) == 0)
            {
                if (g_niki_mode != 0)
                {
                    g_niki_selected_row = 0;
                }
                g_niki_entry_scan_active = 0;
                if (g_niki_entry_state == 0xF8)
                {
                    return phase_result;
                }
                if (g_niki_entry_state == 0xFA)
                {
                    break;
                }
                niki_commit_selected_entry();
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 20);
        break;

    case 8:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_800172AC(g_niki_card_slot * 0x10);
        g_niki_load_step = g_niki_load_step + 1;
        break;

    case 9:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_8001725C(g_niki_card_slot * 0x10);
        g_niki_primary_poll_countdown = 0x10;
        g_niki_secondary_poll_countdown = 0x10;
        g_niki_load_step = g_niki_load_step + 1;
        break;

    case 0:
        phase_result = 2;
        g_niki_progress_active = 0;
        break;

    case 10:
        func_80016F9C(&path, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name);
        wait_attempts = 0;
        func_8001729C(g_niki_card_slot);
        do
        {
            poll_result = func_8001686C(&path);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        } while (wait_attempts < 20);
        goto block_increment;

    case 15:
        poll_result = niki_poll_primary_handle_group();
        if (poll_result >= 3)
        {
            goto check_card_change;
        }
        if (poll_result > 0)
        {
            goto retry_card_ready;
        }
        if (poll_result == 0)
        {
            goto block_increment;
        }
        break;
    check_card_change:
        if (poll_result == 3)
        {
            goto retry_card_change;
        }
        break;
    retry_card_ready:
        g_niki_secondary_poll_countdown = g_niki_secondary_poll_countdown - 1;
        if (g_niki_secondary_poll_countdown != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        g_niki_selection_status = 0;
        g_niki_entry_state = 0xFD;
        break;
    retry_card_change:
        g_niki_primary_poll_countdown = g_niki_primary_poll_countdown - 1;
        if (g_niki_primary_poll_countdown == 0)
        {
            goto card_change_timeout;
        }
    block_reissue:
        func_8001729C(g_niki_card_slot);
        func_800172AC(g_niki_card_slot * 0x10);
        func_8001729C(g_niki_card_slot);
        func_8001725C(g_niki_card_slot * 0x10);
        break;
    card_change_timeout:
        phase_result = 5;
        g_niki_entry_state = 0xFC;
        g_niki_load_step = D_801606D0;
        break;

    case 16:
        do
        {
            poll_result = niki_poll_secondary_handle_group();
        } while (poll_result == -1);
        goto block_increment;

    case 17:
        g_niki_io_busy = 1;
        g_niki_selection_status = 0;
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, 0x8001);
        if (g_niki_file_handle == -1)
        {
            break;
        }
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, &D_80164B98, g_niki_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(g_niki_file_handle);
            break;
        }
        goto block_increment;

    case 18:
        poll_result = niki_poll_primary_handle_group();
        if (poll_result == 0)
        {
            g_niki_io_busy = 0;
            g_niki_selection_status = 1;
            g_niki_load_step = g_niki_load_step + 1;
            func_8001683C(g_niki_file_handle);
            break;
        }
        if (poll_result == -1)
        {
            break;
        }
        g_niki_io_busy = 0;
        func_8001683C(g_niki_file_handle);
    block_status_ff:
        g_niki_entry_state = 0xFF;
        g_niki_load_step = D_801606C8;
        break;

    case 19:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, 0x8001);
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob.bytes, NIKI_SAVE_FILE_BYTES) == -1)
        {
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
            block_dialog_read:
                niki_open_status_dialog(1);
                break;
            }
            break;
        }
        goto block_increment;

    case 20:
        io_result = niki_poll_primary_handle_group();
        if (io_result == 0)
        {
            g_niki_confirm_latch = 0;
            g_niki_load_step = g_niki_load_step + 1;
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
            goto block_dialog_read;
        }
        goto block_decrement_step;

    case 24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(g_niki_card_slot * 0x10) == 1)
            {
                break;
            }
            func_8002054C(0);
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 20);
        if (wait_attempts != 20)
        {
            func_80032174(0, &card_command, &card_result);
            if (card_result == 0)
            {
                goto block_increment;
            }
        }
        niki_open_status_dialog(3);
        break;

    case 27:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_selected_save_path, 0x8001);
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob.bytes, NIKI_SAVE_FILE_BYTES) == -1)
        {
            func_8001683C(g_niki_file_handle);
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
            block_dialog_write_read:
                niki_open_secondary_status_dialog(1);
                break;
            }
            break;
        }
        goto block_increment;

    case 28:
        io_result = niki_poll_primary_handle_group();
        if (io_result == 0)
        {
            g_niki_confirm_latch = 0;
            g_niki_load_step = g_niki_load_step + 1;
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
        goto block_close_decrement;

    case 30:
        g_niki_retry_count = 5;
        g_niki_load_step = g_niki_load_step + 1;
        break;

    case 25:
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
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(&path, 0x20200);
        if (g_niki_file_handle != -1)
        {
            goto block_write_opened;
        }
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
    block_write_retry:
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
        block_dialog_write:
            niki_open_secondary_status_dialog(0);
            break;
        }
        break;

    block_write_opened:
        func_8001683C(g_niki_file_handle);
        func_800170BC(g_niki_temporary_save_path, &path);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(g_niki_temporary_save_path, 0x8002);
        niki_release_primary_handles();
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        if (func_8001682C(g_niki_file_handle, g_niki_save_blob.bytes, NIKI_SAVE_FILE_BYTES) == -1)
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
            goto block_write_retry;
        }
        goto block_increment;

    block_increment:
        g_niki_load_step = g_niki_load_step + 1;
        break;

    case 26:
        io_result = niki_poll_primary_handle_group();
        if (io_result != 0)
        {
            if (io_result < 0)
            {
                break;
            }
            if (io_result >= 4)
            {
                break;
            }
            goto block_case26_retry;
        }
        if (g_niki_preserve_old_save != 0)
        {
            /* The replacement has finished writing; the old save can now be removed. */
            func_8001729C(g_niki_card_slot);
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
        func_8001729C(g_niki_card_slot);
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
        g_niki_load_step = g_niki_load_step + 1;
        func_8001683C(g_niki_file_handle);
        break;
    default:
        return phase_result;

block_case26_retry:
    g_niki_retry_count = g_niki_retry_count - 1;
    if (g_niki_retry_count == 0)
    {
        goto block_case26_exhausted;
    }

block_close_decrement:
    func_8001683C(g_niki_file_handle);
block_decrement_step:
    g_niki_load_step = g_niki_load_step - 1;
    break;

block_case26_exhausted:
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
    }

    return phase_result;
}

/** @brief Wildcard matching all files on memory-card slot zero. */
const char g_niki_entry_header_template[7] __attribute__((aligned(4))) = "bu00:*";

/**
 * @brief Request fresh card information and restart the load sequence.
 * @see decomp.me (100.00%)
 */
void niki_restart_load_sequence(void)
{
    func_8001729C(g_niki_card_slot);
    niki_release_primary_handles();
    func_8001724C(g_niki_card_slot * 0x10);
    g_niki_load_step = D_801606D0;
}

/**
 * @brief Poll software card events and request fresh card information after an event.
 * @return Event index: 0 completion, 1 error, 2 timeout, 3 new card; -1 if none is pending.
 * @see decomp.me (100.00%)
 */
s32 niki_poll_and_rewind_primary_handles(void)
{
    s32 event_index;

    event_index = niki_poll_primary_handle_group();
    if (event_index != -1)
    {
        func_8001729C(g_niki_card_slot);
        func_8001724C(g_niki_card_slot * 0x10);
    }
    return event_index;
}

/**
 * @brief Open and enable software and hardware memory-card events for polling.
 * @see decomp.me (100.00%)
 */
void niki_init_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    g_niki_primary_handle0 = func_800167AC(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_niki_primary_handle1 = func_800167AC(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_niki_primary_handle2 = func_800167AC(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_niki_primary_handle3 = func_800167AC(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    g_niki_secondary_handle0 = func_800167AC(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_niki_secondary_handle1 = func_800167AC(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_niki_secondary_handle2 = func_800167AC(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_niki_secondary_handle3 = func_800167AC(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    func_800167DC(g_niki_primary_handle0);
    func_800167DC(g_niki_primary_handle1);
    func_800167DC(g_niki_primary_handle2);
    func_800167DC(g_niki_primary_handle3);
    func_800167DC(g_niki_secondary_handle0);
    func_800167DC(g_niki_secondary_handle1);
    func_800167DC(g_niki_secondary_handle2);
    func_800167DC(g_niki_secondary_handle3);
    func_800167FC();
    g_niki_progress_bar_active = 0;
    g_niki_entry_scan_active = 0;
}

/**
 * @brief Close all software and hardware memory-card event handles.
 * @see decomp.me (100.00%)
 */
void niki_shutdown_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(g_niki_primary_handle0);
    func_800167BC(g_niki_primary_handle1);
    func_800167BC(g_niki_primary_handle2);
    func_800167BC(g_niki_primary_handle3);
    func_800167BC(g_niki_secondary_handle0);
    func_800167BC(g_niki_secondary_handle1);
    func_800167BC(g_niki_secondary_handle2);
    func_800167BC(g_niki_secondary_handle3);
    func_800167FC();
}

/**
 * @brief Reset the browser and read the selected card's first directory entry.
 * @param card_slot Memory-card slot to scan.
 * @return One if the first entry was read, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_begin_entry_scan(s32 card_slot)
{
    NikiDirectoryPattern pattern;

    memcpy(&pattern, &g_niki_entry_header_template, 7);
    g_niki_selected_row = 0;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_entry_state = 0;
    pattern.device.characters.slot += card_slot;
    if (func_80016BCC(&pattern, g_niki_entries[card_slot]) != 0)
    {
        func_800B0170(&g_niki_entries[card_slot][g_niki_entry_state]);
        g_niki_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Read the next directory entry, or rank the completed directory.
 * @param page Memory-card slot being scanned.
 * @return One if another directory entry was read, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_scan_next_entry(s32 page)
{
    s32 entry_index;
    s32 used_blocks;
    s32 selected;
    s32 entry_count;
    s32 card_full;

    if (func_8001684C(&g_niki_entries[page][g_niki_entry_state]) != 0)
    {
        func_800B0170(&g_niki_entries[page][g_niki_entry_state]);
        g_niki_entry_state += 1;
        return 1;
    }

    func_800AA02C();
    if ((g_niki_mode == 0) && (niki_has_known_entry_type() == 0))
    {
        g_niki_entry_state = 0xF8;
    }
    else
    {
        entry_index = 0;
        used_blocks = 0;
        g_niki_preserve_old_save = 0;
        entry_count = g_niki_entry_state;
        if (entry_count > 0)
        {
            u8* entries;
            s32 offset;
            do
            {
                entries = (u8*)g_niki_entries;
            } while (0);
            offset = g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES;
            do
            {
                used_blocks += ((NikiDirEntry*)(offset + (s32)entries))->size / NIKI_MEMORY_CARD_BLOCK_BYTES;
                entry_index++;
                offset += NIKI_DIRECTORY_ENTRY_BYTES;
            } while (entry_index < entry_count);
        }
        card_full = used_blocks >= 0xE;
        if (card_full != 0)
        {
            selected = niki_rank_entries(used_blocks, entry_index, entry_count);
            if (niki_has_known_entry_type() == 0)
            {
                g_niki_entry_state = 0xFA;
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
            selected = niki_rank_entries(used_blocks, entry_index, entry_count);
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
 * @see decomp.me (100.00%)
 */
void niki_commit_selected_entry(void)
{
    NikiSelectedFilePath path;
    u8* path_bytes;

    if (g_niki_entry_state == 0)
    {
        g_niki_selection_status = 3;
        return;
    }
    {
        if (func_8001714C(&D_800ECFC4[0], g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 8) == 0)
        {
            g_niki_selection_status = 2;
            return;
        }
    }
    memcpy(&path, &g_niki_file_template, 6);
    path_bytes = (u8*)&path;
    {
        func_80016F9C(path_bytes, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name);
    }
    {
        s32 slot;
        s32 value;
        value = path.device.characters.slot;
        slot = (u8)g_niki_card_slot;
        g_niki_selection_status = 0;
        value += slot;
        path.device.characters.slot = value;
        func_800170BC(&g_niki_selected_save_path[0], path_bytes, slot);
    }
    g_niki_load_step = &D_801606E0[0];
    {
        if (func_8001714C(&D_800ECF7C[0], g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 0xC) == 0)
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

/**
 * @brief Consume pending software memory-card events.
 * @see decomp.me (100.00%)
 */
void niki_release_primary_handles(void)
{
    func_800167CC(g_niki_primary_handle0);
    func_800167CC(g_niki_primary_handle1);
    func_800167CC(g_niki_primary_handle2);
    func_800167CC(g_niki_primary_handle3);
}

/**
 * @brief Consume pending hardware memory-card events.
 * @see decomp.me (100.00%)
 */
void niki_release_secondary_handles(void)
{
    func_800167CC(g_niki_secondary_handle0);
    func_800167CC(g_niki_secondary_handle1);
    func_800167CC(g_niki_secondary_handle2);
    func_800167CC(g_niki_secondary_handle3);
}

/**
 * @brief Consume the first pending software card event in priority order.
 * @return Event index: 0 completion, 1 error, 2 timeout, 3 new card; -1 if none is pending.
 * @see decomp.me (100.00%)
 */
s32 niki_poll_primary_handle_group(void)
{
    if (func_800167CC(g_niki_primary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_niki_primary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_niki_primary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_niki_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Consume the first pending hardware card event in priority order.
 * @return Event index: 0 completion, 1 error, 2 timeout, 3 new card; -1 if none is pending.
 * @see decomp.me (100.00%)
 */
s32 niki_poll_secondary_handle_group(void)
{
    if (func_800167CC(g_niki_secondary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_niki_secondary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_niki_secondary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_niki_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Group recognized save-file types by suffix, then append other entries.
 * @note Preserves directory order within each type and suffix group.
 */
void niki_sort_entries_by_type(void)
{
    NikiDirEntry sorted[NIKI_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
        {
            if (g_niki_entry_suffix_values[entry_index] == suffix && func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
        {
            if (g_niki_entry_suffix_values[entry_index] == suffix && func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECFC4, g_niki_entries[g_niki_card_slot][entry_index].name, 8) == 0)
        {
            func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) != 0 &&
            func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) != 0 &&
            func_8001714C(D_800ECFC4, g_niki_entries[g_niki_card_slot][entry_index].name, 8) != 0)
        {
            func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        func_80016E7C(&sorted[entry_index], &g_niki_entries[g_niki_card_slot][entry_index], sizeof(NikiDirEntry));
    }
}

/**
 * @brief Draw a signed decimal value, suppressing leading zeroes.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param value Number to display, with magnitude at most 99999.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @param palette Glyph palette index.
 * @param alignment Text alignment relative to the anchor.
 * @return GPU packet cursor after the text packets.
 */
s32 niki_draw_signed_decimal(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 glyphs[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    glyphs[1] = g_niki_decimal_glyphs[magnitude / 10000];
    glyphs[2] = g_niki_decimal_glyphs[(magnitude % 10000) / 1000];
    glyphs[3] = g_niki_decimal_glyphs[(magnitude % 1000) / 100];
    glyphs[4] = g_niki_decimal_glyphs[(magnitude % 100) / 10];
    glyphs[5] = g_niki_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    glyphs[6] = 0;

    while (first_digit < 5 && glyphs[first_digit] == NIKI_SJIS_FULLWIDTH_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        glyphs[first_digit] = NIKI_SJIS_MINUS;
    }
    prim = niki_draw_cached_text(prim, ot, (u8*)&glyphs[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Draw a byte as two hexadecimal glyphs.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param value Byte value to display, from 0 through 255.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @param alignment Text alignment relative to the anchor.
 */
void niki_draw_hex_byte(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 glyphs[3];
    s32 high_digit;
    s32 low_digit;
    u16* high_glyph;

    high_digit = value / 16;
    high_glyph = &g_niki_hex_glyphs[high_digit];
    low_digit = value % 16;
    glyphs[0] = *high_glyph;
    glyphs[1] = g_niki_hex_glyphs[low_digit];
    glyphs[2] = 0;
    niki_draw_cached_text(prim, ot, (u8*)glyphs, x, y, 0, alignment);
}

/**
 * @brief Draw text through the glyph cache and append its texture-page command.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param text Text containing one-byte characters and Shift-JIS pairs.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @param palette Glyph palette index.
 * @param alignment Zero for left, one for right, or two for centered alignment.
 * @return GPU packet cursor after the text and draw-mode packets.
 */
s32 niki_draw_cached_text(s32 prim, s32* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            code = *scan;
            if (code >= 0x80)
            {
                scan++;
            }
            scan++;
            count++;
        } while (*scan >= 0x20);
    }

    switch (alignment)
    {
    case 1:
        x -= count * 0x10;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    g_niki_text_line_start_x = x;
    g_niki_glyph_cursor_x = x;
    g_niki_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_niki_glyph_cursor_x += 0x10;
            continue;
        }
        if ((u8)lead >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if ((u8)lead < 0x20)
            {
                break;
            }
            if ((u32)(lead - 0x30) < 0x50)
            {
                code = *cursor - 0x7DE1;
                cursor++;
            }
            else
            {
                code = *cursor - 0x7AE1;
                cursor++;
            }
        }
        prim = niki_render_cached_glyph(prim, ot, code, palette);
    }

    NIKI_SET_PACKET_LENGTH(prim, 1);
    ((NikiDrawModePacket*)prim)->command = 0xE1000005;
    NIKI_ADD_PRIMITIVE(ot, prim);
    return (s32)((NikiDrawModePacket*)prim + 1);
}

s32 niki_render_cached_glyph(s32 prim, s32* ot, s32 character_code, s32 palette)
{
    NikiGlyphCacheEntry* entry;
    u8* font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8* raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8* raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = g_niki_glyph_cache;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return niki_emit_glyph_sprite((NikiGlyphSprite*)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = func_8001687C(code & 0xFFFF);
    font_data = (u8*)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = g_niki_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? color_index : 0;

                mask >>= 1;
                high_pixel_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (high_pixel_set)
                {
                    packed_pixels += high_nibble_color;
                }

                *raster_byte = packed_pixels;

                mask >>= 1;
                raster++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < GLYPH_CACHE_SLOTS) && (g_niki_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_niki_glyph_cache[slot].raw = code & 0xFFFF;
    prim = niki_emit_glyph_sprite((NikiGlyphSprite*)prim, ot, slot, palette);

    g_niki_glyph_upload_x = (slot % GLYPH_CACHE_COLUMNS) * 4;
    g_niki_glyph_upload_y = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_niki_glyph_upload_x + 0x140;
    rect.y = g_niki_glyph_upload_y;

    func_80019A34(&rect, g_niki_glyph_raster_cursor);
    func_80019788(0);

    g_niki_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}

s32 niki_emit_glyph_sprite(NikiGlyphSprite* sprite, s32* ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_niki_glyph_cache[cache_slot].raw |= 0x10000;

    NIKI_SET_PACKET_LENGTH(sprite, 3);
    NIKI_SET_PACKET_CODE(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = g_niki_glyph_cursor_x;
    sprite->packet.y0 = g_niki_glyph_cursor_y;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->packet.u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->packet.v0 = cache_slot & GLYPH_CACHE_ROW_MASK;
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = g_niki_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_niki_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_niki_glyph_cursor_x = g_niki_text_line_start_x;
        g_niki_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}

/**
 * @brief Start a frame with all cached glyphs marked unused and reset raster allocation.
 * @see decomp.me (100.00%)
 */
void niki_begin_glyph_cache_frame(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;

    g_niki_glyph_raster_cursor = g_niki_glyph_raster_buffer;
    slot = 0;
    entry = g_niki_glyph_cache;
    for (; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        entry->raw = (u16)entry->raw;
    }
}

/**
 * @brief Free cache slots whose glyphs were not drawn this frame.
 * @see decomp.me (100.00%)
 */
void niki_evict_unused_glyphs(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;
    s32 used_flag;

    slot = 0;
    used_flag = 0x10000;
    entry = g_niki_glyph_cache;
    for (; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        if (!(entry->raw & used_flag))
        {
            entry->raw = 0;
        }
    }
}

/**
 * @brief Clear cached character codes and the glyph raster buffer.
 * @see decomp.me (100.00%)
 */
void niki_reset_glyph_cache(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;

    slot = GLYPH_CACHE_SLOTS - 1;
    entry = g_niki_glyph_cache;
    entry += GLYPH_CACHE_SLOTS - 1;
    for (; slot >= 0; slot--, entry--)
    {
        entry->raw = 0;
    }

    slot = 0;
    for (; slot < GLYPH_CACHE_SLOTS * GLYPH_RASTER_BYTES; slot++)
    {
        g_niki_glyph_raster_buffer[slot] = 0;
    }
}

/*
 * The extended table linker symbol is biased backwards by 0x19 pages.  This
 * lets the original code index it directly with the encoded lead byte
 * (0x19..0x1F) instead of subtracting NIKI_TEXT_EXTENDED_LEAD_FIRST first.
 */

/**
 * @brief Expand NIKI's internal text encoding into a NUL-terminated Shift-JIS
 *        byte string.
 * @param dst_sjis Destination buffer. Each decoded source character writes one
 *        two-byte Shift-JIS code; the function appends a single NUL byte.
 * @param src_text NUL-terminated NIKI text. Bytes 0x19..0x1F introduce a
 *        two-byte table code; printable one-byte codes use the compact table.
 * @note Each lookup row contains 16 two-byte Shift-JIS codes followed by a
 *       newline byte, so sizeof(NikiSjisRow) is 33 and sizeof(NikiSjisPage) is
 *       528.
 * @see decomp.me (100.00%)
 */
void niki_expand_text_glyph_codes(u8* dst_sjis, const u8* src_text)
{
    u32 source_byte;
    s32 glyph_index;
    s16 lead_byte;

    for (;;)
    {
        source_byte = *src_text;
        if ((u8)source_byte != 0)
        {
            if ((u32)(source_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
            {
                u32 trail_byte;
                s32 row;
                u8* sjis_lead;
                u8* sjis_trail;

                trail_byte = src_text[1];
                row = trail_byte >> NIKI_SJIS_ROW_SHIFT;
                trail_byte &= NIKI_SJIS_CODES_PER_ROW - 1;
                sjis_lead = (u8*)g_niki_double_byte_char_table + trail_byte * sizeof(NikiSjisCode);
                sjis_lead += row * sizeof(NikiSjisRow);
                lead_byte = *src_text;
                sjis_lead += lead_byte * sizeof(NikiSjisPage);
                *dst_sjis = *sjis_lead;
                dst_sjis++;

                trail_byte = src_text[1];
                row = trail_byte >> NIKI_SJIS_ROW_SHIFT;
                trail_byte &= NIKI_SJIS_CODES_PER_ROW - 1;
                sjis_trail = (u8*)g_niki_double_byte_char_table + 1 + trail_byte * sizeof(NikiSjisCode);
                sjis_trail += row * sizeof(NikiSjisRow);
                lead_byte = *src_text;
                sjis_trail += lead_byte * sizeof(NikiSjisPage);
                *dst_sjis = *sjis_trail;
                dst_sjis++;
                src_text += 2;
            }
            else if ((u8)source_byte >= NIKI_TEXT_PRINTABLE_FIRST)
            {
                lead_byte = *src_text;
                glyph_index = lead_byte - NIKI_TEXT_SINGLE_BYTE_BASE;
                *dst_sjis = ((u8*)g_niki_single_byte_char_table)[(glyph_index / NIKI_SJIS_CODES_PER_ROW) * sizeof(NikiSjisRow) +
                                                                 (glyph_index & (NIKI_SJIS_CODES_PER_ROW - 1)) * sizeof(NikiSjisCode)];
                dst_sjis++;

                lead_byte = *src_text;
                glyph_index = lead_byte - NIKI_TEXT_SINGLE_BYTE_BASE;
                *dst_sjis = ((u8*)g_niki_single_byte_char_table)[(glyph_index / NIKI_SJIS_CODES_PER_ROW) * sizeof(NikiSjisRow) +
                                                                 (glyph_index & (NIKI_SJIS_CODES_PER_ROW - 1)) * sizeof(NikiSjisCode) + 1];
                dst_sjis++;
                src_text += 1;
            }
            else
            {
                *dst_sjis = g_niki_single_byte_char_table[0].codes[0].lead;
                dst_sjis++;
                *dst_sjis = g_niki_single_byte_char_table[0].codes[0].trail;
                dst_sjis++;
                src_text += 1;
            }
        }
        else
        {
            *dst_sjis = 0;
            return;
        }
    }
}
