#include "internal/cload_internal.h"
#include <memory.h>
#include <strings.h>
#include <fcntl.h>
#include <libetc.h>
#include "main/controller.h"
#include "overlays/field/field_input.h"

void *bcopy(const unsigned char *src, unsigned char *dst, int count);

/**
 * @brief Report "no memory card" and ask the caller to refresh the entry list.
 * @param result Phase-result variable that receives CARD_MENU_SEQUENCE_NO_CARD.
 */
#define CLOAD_REQUEST_REFRESH(result)                                                                                                                          \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (result) = CARD_MENU_SEQUENCE_NO_CARD;                                                                                                                         \
        g_cload_selection_status = 0;                                                                                                                          \
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;                                                                                                                             \
    } while (0)

/**
 * @brief Finish the current step table with a result, an entry status and a next table.
 * @param result Phase-result variable that receives @p code.
 * @param code CardMenuSequenceResult to return to the caller.
 * @param entry_state New g_card_entry_state status code.
 * @param next_step New g_cload_load_step table (NULL stops the sequence).
 */
#define CLOAD_END_STEP_TABLE(result, code, entry_state, next_step)                                                                                             \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (result) = (code);                                                                                                                                     \
        g_card_entry_state = (entry_state);                                                                                                                    \
        g_cload_load_step = (next_step);                                                                                                                       \
    } while (0)

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"
#include "../../common/save_file/format_hex.inc.c"
#include "../../common/save_file/hex_nibble_to_ascii.inc.c"
#include "../../common/save_file/parse_hex.inc.c"
#include "../../common/save_file/parse_hex_suffix_byte.inc.c"
#include "../../common/card_directory/parse_entry_fields.inc.c"

/**
 * @brief Rank the current page's entries and select the highest-scoring slot.
 * @return Index of the entry holding the maximum value.
 */
s32 cload_rank_entries(void)
{
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;

    parse_entry_fields();
    maximum = -1;
    cload_sort_entries_by_type();
    max_suffix = parse_entry_fields();
    cload_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_entry_fields[g_card_slot][entry_index] >= 0)
        {
            if (g_card_entry_fields[g_card_slot][entry_index] >= maximum)
            {
                g_cload_entry_ranks[entry_index] = next_rank;
                maximum = g_card_entry_fields[g_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_card_entry_fields[g_card_slot][entry_index] < g_card_entry_fields[g_card_slot][previous_index])
                    {
                        higher_count++;
                        g_cload_entry_ranks[previous_index]++;
                    }
                }
                g_cload_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_cload_rank_count = next_rank;
    /* Reuse next_rank as the running maximum and maximum as its index. */
    next_rank = -1;
    maximum = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (next_rank < g_card_entry_fields[g_card_slot][entry_index])
        {
            next_rank = g_card_entry_fields[g_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    g_cload_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, 8) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix + 1;
            break;
        }
    }
    return maximum;
}

/**
 * @brief Reset the cload menu state: set the row-count/pitch field to 0x28 and
 *        clear all 15 slot entries of g_cload_entry_ranks to -1 (empty).
 */
void cload_reset_entry_ranks(void)
{
    s32 rank_index;
    s32 empty_rank;

    g_cload_rank_count = 0x28;
    empty_rank = -1;
    for (rank_index = 14; rank_index >= 0; rank_index--)
    {
        g_cload_entry_ranks[rank_index] = empty_rank;
    }
}

/**
 * @brief Scan up to g_card_entry_state entries of the g_card_entries table (row
 *        selected by g_card_slot, stride 0x28) and report whether any entry
 *        matches one of the two known-type patterns g_lom_save_filename_prefix / g_lom_pocketstation_filename_prefix.
 * @return 1 on the first entry that matches either pattern (strncmp returns 0
 *         on a match), 0 if no entry matches.
 */
s32 cload_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0 ||
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Check whether the current card's directory entries use at least 14 blocks.
 * @return 1 if the summed block count is >= 14, otherwise 0.
 * @note Inlined into cload_scan_next_entry.
 */
inline s32 cload_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        used_blocks += g_card_entries[g_card_slot][entry_index].size / CARD_BLOCK_BYTES;
    }
    return used_blocks >= 14;
}

/**
 * @brief Erase the two fixed per-slot memory-card files.
 * @note Each erase starts from the six-byte "bu00:" device path, adjusts the
 *       slot digit, appends one fixed filename suffix, and calls Psy-Q erase().
 * @note Inlined into cload_advance_load_sequence.
 */
inline void cload_erase_fixed_card_files(void)
{
    CardFilePath card_path;

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += (u8)g_card_slot;
    strcat(card_path.text, g_lom_save_dummy_filename);
    erase(&card_path);

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += (u8)g_card_slot;
    strcat(card_path.text, g_lom_pocketstation_dummy_filename);
    erase(&card_path);
}

/**
 * @brief Run the current memory-card load step and advance g_cload_load_step.
 * @return CardMenuSequenceResult phase code for cload_update_load_sequence.
 * @note g_cload_load_step walks one of the g_cload_steps_* byte tables; each
 *       CardMenuStep opcode issues or polls a card command, reads the selected
 *       save, or scans the card directory, and updates g_card_entry_state /
 *       g_cload_selection_status. Opcodes with no case are no-ops.
 */
s32 cload_advance_load_sequence(void)
{
    CardSequencePath card_path;
    long status0;
    long status1;
    s32 phase_result;
    s32 scan_attempts;
    s32 wait_attempts;
    s32 rank_index;
    s32 rank_fill;
    s32 poll_result;

    /* Builds the "bu00:" slot path like the erase helper; never used afterwards. */
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    phase_result = CARD_MENU_SEQUENCE_WAIT;
    card_path.device.characters.slot += *(u8*)&g_card_slot;

    if (g_cload_load_step != NULL)
    {
        switch (*g_cload_load_step)
        {
        case CARD_MENU_STEP_CARD_INFO:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_info(g_card_slot * 0x10);
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_POLL_CARD_INFO:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_cload_load_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                phase_result = CARD_MENU_SEQUENCE_NO_CARD;
                g_cload_selection_status = 0;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                g_cload_load_step++;
                cload_deactivate_primary_element();
                break;
            case CARD_EVENT_NEW_CARD:
                g_cload_rank_count = 0x28;
                rank_fill = -1;
                for (rank_index = 14; rank_index >= 0; rank_index--)
                {
                    g_cload_entry_ranks[rank_index] = rank_fill;
                }
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_cload_load_step = g_cload_steps_initial_scan;
                break;
            }
            break;

        case CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS:
            clear_software_card_events();
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_POLL_HARDWARE_EVENTS:
            do
            {
                poll_result = poll_hardware_card_events();
            } while (poll_result == -1);
            switch (poll_result)
            {
            case CARD_EVENT_COMPLETE:
                g_cload_load_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                CLOAD_REQUEST_REFRESH(phase_result);
                break;
            }
            break;

        case CARD_MENU_STEP_CLEAR_HARDWARE_EVENTS:
            clear_hardware_card_events();
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_SCAN_ENTRIES:
            cload_erase_fixed_card_files();
            g_cload_entry_scan_active = 1;
            if (cload_begin_entry_scan(g_card_slot) == 0)
            {
                CLOAD_END_STEP_TABLE(phase_result, CARD_MENU_SEQUENCE_FINISHED, CARD_MENU_ENTRY_STATE_NO_GAME_DATA, NULL);
                g_cload_entry_scan_active = 0;
                break;
            }
            scan_attempts = 0;
            g_cload_load_step++;
            do
            {
                if (cload_scan_next_entry(g_card_slot) == 0)
                {
                    g_cload_entry_scan_active = 0;
                    if (g_card_entry_state != CARD_MENU_ENTRY_STATE_NO_GAME_DATA && g_card_entry_state != CARD_MENU_ENTRY_STATE_CARD_FULL)
                    {
                        cload_commit_selected_entry();
                    }
                    break;
                }
                scan_attempts++;
            } while (scan_attempts < CARD_MENU_FILE_OP_ATTEMPTS);
            break;

        case CARD_MENU_STEP_CLEAR_CARD:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_clear(g_card_slot * 0x10);
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_LOAD_CARD:
            phase_result = CARD_MENU_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_load(g_card_slot * 0x10);
            g_cload_primary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
            g_cload_secondary_poll_countdown = CARD_MENU_CARD_LOAD_RETRIES;
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_DONE:
            phase_result = CARD_MENU_SEQUENCE_FINISHED;
            D_80162370 = 0;
            break;

        case CARD_MENU_STEP_POLL_CARD_LOAD:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_cload_load_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                g_cload_secondary_poll_countdown--;
                if (g_cload_secondary_poll_countdown == 0)
                {
                    CLOAD_REQUEST_REFRESH(phase_result);
                }
                else
                {
                    _card_wait(g_card_slot);
                    _card_clear(g_card_slot * 0x10);
                    _card_wait(g_card_slot);
                    _card_load(g_card_slot * 0x10);
                }
                break;
            case CARD_EVENT_NEW_CARD:
                g_cload_primary_poll_countdown--;
                if (g_cload_primary_poll_countdown != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(g_card_slot * 0x10);
                    _card_wait(g_card_slot);
                    _card_load(g_card_slot * 0x10);
                }
                else
                {
                    CLOAD_END_STEP_TABLE(phase_result, CARD_MENU_SEQUENCE_UNFORMATTED, CARD_MENU_ENTRY_STATE_NO_SAVE_DATA,
                                         g_cload_steps_idle);
                }
                break;
            }
            break;

        case CARD_MENU_STEP_WAIT_HARDWARE_EVENTS:
            do
            {
                poll_result = poll_hardware_card_events();
            } while (poll_result == -1);
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_READ_ENTRY:
            g_cload_io_busy = 1;
            g_cload_selection_status = 0;
            _card_wait(g_card_slot);
            g_cload_file_handle = open(g_cload_selected_card_path, FASYNC | FREAD);
            if (g_cload_file_handle == -1)
            {
                break;
            }
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_cload_file_handle, &g_cload_selected_file,
                     g_cload_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : CARD_MENU_ENTRY_TITLE_READ_BYTES) != -1)
            {
                g_cload_load_step++;
            }
            else
            {
                close(g_cload_file_handle);
            }
            break;

        case CARD_MENU_STEP_POLL_ENTRY_READ:
            poll_result = poll_software_card_events();
            if (poll_result == CARD_EVENT_COMPLETE)
            {
                g_cload_io_busy = 0;
                g_cload_selection_status = 1;
                g_cload_load_step++;
                close(g_cload_file_handle);
            }
            else if (poll_result != -1)
            {
                g_cload_io_busy = 0;
                close(g_cload_file_handle);
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_cload_load_step = g_cload_steps_initial_scan;
            }
            break;

        case CARD_MENU_STEP_INIT_RETRIES:
            g_cload_retry_count = CARD_MENU_SAVE_RETRIES;
            g_cload_load_step++;
            break;

        case CARD_MENU_STEP_READ_SAVE:
            g_cload_progress_active = 1;
            g_cload_progress_bar_active = 1;
            g_cload_progress_start_tick = VSync(-1);
            _card_wait(g_card_slot);
            g_cload_file_handle = open(g_cload_selected_card_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_cload_file_handle, &g_cload_save_file, sizeof(g_cload_save_file)) != -1)
            {
                g_cload_load_step++;
                break;
            }
            close(g_cload_file_handle);
            g_cload_retry_count--;
            if (g_cload_retry_count == 0)
            {
                cload_open_status_dialog(1);
            }
            break;

        case CARD_MENU_STEP_POLL_SAVE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_cload_progress_active = 0;
                g_cload_load_step++;
                close(g_cload_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_cload_retry_count--;
                if (g_cload_retry_count != 0)
                {
                    /* Rewind to CARD_MENU_STEP_READ_SAVE and try again. */
                    close(g_cload_file_handle);
                    g_cload_load_step--;
                    break;
                }
                close(g_cload_file_handle);
                g_cload_progress_bar_active = 0;
                cload_open_status_dialog(1);
                break;
            }
            break;

        case CARD_MENU_STEP_CHECK_POCKETSTATION:
            for (wait_attempts = 0; wait_attempts < CARD_MENU_FILE_OP_ATTEMPTS; wait_attempts++)
            {
                if (McxCardType(g_card_slot * 0x10) == MCX_COMMAND_ISSUED)
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
                    g_cload_load_step++;
                    break;
                }
            }
            cload_open_status_dialog(3);
            break;
        }
    }
    return phase_result;
}

/**
 * @brief Reset the cached resource handles and arm the first load step.
 * @note Releases the handles (clear_software_card_events), rewinds the CD channel, and points
 *       g_cload_load_step at the g_cload_steps_idle step table.
 */
void cload_restart_load_sequence(void)
{
    clear_software_card_events();
    _card_wait(g_card_slot);
    _card_info(g_card_slot * 0x10);
    g_cload_load_step = g_cload_steps_idle;
}

#include "../../common/card_events/poll_and_retry_card_info.inc.c"

/**
 * @brief Allocate and register the eight streaming buffers for this overlay.
 * @note Brackets the eight OpenEvent allocations (handles stored in
 *       g_card_software_event_io_complete..g_card_hardware_event_new_card) with EnterCriticalSection / ExitCriticalSection and resets the
 *       stream bookkeeping (g_cload_entry_scan_active, g_cload_progress_start_tick, g_cload_progress_bar_active).
 */
void cload_init_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_card_software_event_io_complete = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_card_software_event_error = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_card_software_event_timeout = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_card_software_event_new_card = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_card_hardware_event_io_complete = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_card_hardware_event_error = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_card_hardware_event_timeout = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_card_hardware_event_new_card = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    EnableEvent(g_card_software_event_io_complete);
    EnableEvent(g_card_software_event_error);
    EnableEvent(g_card_software_event_timeout);
    EnableEvent(g_card_software_event_new_card);
    EnableEvent(g_card_hardware_event_io_complete);
    EnableEvent(g_card_hardware_event_error);
    EnableEvent(g_card_hardware_event_timeout);
    EnableEvent(g_card_hardware_event_new_card);
    ExitCriticalSection();
    g_cload_entry_scan_active = 0;
    g_cload_progress_start_tick = VSync(-1);
    g_cload_progress_bar_active = 0;
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
    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_card_entry_state = 0;
    search_path.device.characters.slot += page;
    if (firstfile(&search_path, &g_card_entries[page][0]) != 0)
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
 * @note When the directory is complete, cload_entry_blocks_reach_limit decides
 *       whether a full card clamps the state (0xFA) or the selection
 *       (g_cload_selected_row) is set from cload_rank_entries's result.
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
    if (cload_has_known_entry_type() == 0)
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        if (cload_entry_blocks_reach_limit())
        {
            selected = cload_rank_entries();
            if (cload_has_known_entry_type() == 0)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
                g_cload_entry_value_limit = 0;
            }
            else
            {
                g_cload_selected_row = selected;
                cload_scroll_to_selection();
            }
        }
        else
        {
            selected = cload_rank_entries();
            if (cload_has_known_entry_type() == 0)
            {
                g_cload_selected_row = 0;
                cload_scroll_to_selection();
                g_cload_entry_value_limit = 0;
            }
            else
            {
                g_cload_selected_row = selected;
                cload_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Commit the selected g_card_entries record and arm the next step.
 * @note Rejects the new-save placeholder entry, builds "buX0:<name>" for the
 *       selected entry, copies it to g_cload_selected_card_path, and flags
 *       whether the entry is an extended (g_lom_save_filename_prefix) save.
 */
void cload_commit_selected_entry(void)
{
    CardEntryPath card_path;

    if (g_card_entry_state == 0)
    {
        g_cload_selection_status = 3;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_cload_selected_row].name, 8) == 0)
    {
        g_cload_selection_status = 2;
        return;
    }
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    strcat(card_path.text, g_card_entries[g_card_slot][g_cload_selected_row].name);
    card_path.device.characters.slot += (u8)g_card_slot;
    g_cload_selection_status = 0;
    strcpy(g_cload_selected_card_path, card_path.text);
    g_cload_load_step = &g_cload_steps_read_selected_header[0];
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_cload_selected_row].name, 0xC) == 0)
    {
        g_cload_selected_entry_extended = 1;
        return;
    }
    g_cload_selected_entry_extended = 0;
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

/**
 * @brief Collate the g_card_entries page records, ordering them by pattern class.
 * @note Five passes bucket records matching g_lom_save_filename_prefix, then g_lom_pocketstation_filename_prefix, then
 *       g_new_save_entry_prefix, then the remainder, copying each 0x28-byte record with
 *       bcopy before writing the ordered set back to the page.
 */
void cload_sort_entries_by_type(void)
{
    struct DIRENTRY sorted_entries[CARD_DIRECTORY_ENTRY_COUNT];
    s32 output_count = 0;
    s32 group;
    s32 entry_index;

    for (group = 0; group < CARD_MENU_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == group &&
                strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0)
            {
                bcopy(&g_card_entries[g_card_slot][entry_index], &sorted_entries[output_count], sizeof(struct DIRENTRY));
                output_count++;
            }
        }
    }

    for (group = 0; group < CARD_MENU_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == group &&
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0)
            {
                bcopy(&g_card_entries[g_card_slot][entry_index], &sorted_entries[output_count], sizeof(struct DIRENTRY));
                output_count++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, 8) == 0)
        {
            bcopy(&g_card_entries[g_card_slot][entry_index], &sorted_entries[output_count], sizeof(struct DIRENTRY));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) != 0 &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) != 0 &&
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, 8) != 0)
        {
            bcopy(&g_card_entries[g_card_slot][entry_index], &sorted_entries[output_count], sizeof(struct DIRENTRY));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        bcopy(&sorted_entries[entry_index], &g_card_entries[g_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}
