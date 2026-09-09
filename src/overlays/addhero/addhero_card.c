#include "addhero_internal.h"

/** @brief Directory search pattern, including the card device and wildcard. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[12];
} AddheroDirectoryPattern;

/** @brief Buffer for the selected save file's complete card path. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[252];
} AddheroSelectedFilePath;

extern AddheroCardPathTemplate g_addhero_entry_header_template;
extern s32 g_addhero_software_event_io_complete;
extern s32 g_addhero_software_event_error;
extern s32 g_addhero_software_event_timeout;
extern s32 g_addhero_software_event_new_card;
extern s32 g_addhero_hardware_event_io_complete;
extern s32 g_addhero_hardware_event_error;
extern s32 g_addhero_hardware_event_timeout;
extern s32 g_addhero_hardware_event_new_card;
extern u8 g_addhero_loadseq_file_ready[];

s32 firstfile(void* a, void* b);
void func_800B0170(void* a);
s32 nextfile(void* a);
void reset_controller_vsync_state(void);
s32 OpenEvent(s32 a, s32 b, s32 c, s32 d);
void CloseEvent(s32 a);
s32 TestEvent(s32 a);
void EnableEvent(s32 a);
void EnterCriticalSection(void);
void ExitCriticalSection(void);

/**
 * @brief Rewind the active card and restart the load sequence from its first
 *        step.
 * @see decomp.me (100.00%)
 */
void addhero_restart_load_sequence(void)
{
    _card_wait(g_addhero_card_slot);
    addhero_clear_software_card_events();
    _card_info(g_addhero_card_slot * 0x10);
    g_addhero_load_step = g_addhero_loadseq_card;
}

/**
 * @brief Poll software card events and request card information again when an
 *        event has arrived.
 * @return The event index (0-3), or -1 when no event has arrived.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_and_retry_card_info(void)
{
    s32 event_status;

    event_status = addhero_poll_software_card_events();
    if (event_status != -1)
    {
        _card_wait(g_addhero_card_slot);
        _card_info(g_addhero_card_slot * 0x10);
    }
    return event_status;
}

/**
 * @brief Register and enable software and hardware memory-card events, then
 *        clear the progress and scan flags.
 * @see decomp.me (100.00%)
 */
void addhero_init_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_addhero_software_event_io_complete = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_addhero_software_event_error = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_addhero_software_event_timeout = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_addhero_software_event_new_card = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    g_addhero_hardware_event_io_complete = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_addhero_hardware_event_error = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_addhero_hardware_event_timeout = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_addhero_hardware_event_new_card = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    EnableEvent(g_addhero_software_event_io_complete);
    EnableEvent(g_addhero_software_event_error);
    EnableEvent(g_addhero_software_event_timeout);
    EnableEvent(g_addhero_software_event_new_card);
    EnableEvent(g_addhero_hardware_event_io_complete);
    EnableEvent(g_addhero_hardware_event_error);
    EnableEvent(g_addhero_hardware_event_timeout);
    EnableEvent(g_addhero_hardware_event_new_card);
    ExitCriticalSection();
    g_addhero_progress_bar_active = 0;
    g_addhero_entry_scan_active = 0;
}

/**
 * @brief Close the software and hardware memory-card events.
 * @see decomp.me (100.00%)
 */
void addhero_shutdown_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_addhero_software_event_io_complete);
    CloseEvent(g_addhero_software_event_error);
    CloseEvent(g_addhero_software_event_timeout);
    CloseEvent(g_addhero_software_event_new_card);
    CloseEvent(g_addhero_hardware_event_io_complete);
    CloseEvent(g_addhero_hardware_event_error);
    CloseEvent(g_addhero_hardware_event_timeout);
    CloseEvent(g_addhero_hardware_event_new_card);
    ExitCriticalSection();
}

/**
 * @brief Reset browser state and read the first directory entry of the given
 *        card page, priming the scan.
 * @param page Card page index to begin scanning.
 * @return 1 if a first entry was read, 0 if the page is empty.
 * @see decomp.me (100.00%)
 */
s32 addhero_begin_entry_scan(s32 page)
{
    AddheroDirectoryPattern buf;

    memcpy(&buf, &g_addhero_entry_header_template, 7);
    g_addhero_selected_row = 0;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_entry_state = 0;
    buf.device.characters.slot += page;
    if (firstfile(&buf, &g_addhero_entries[page][0]) != 0)
    {
        func_800B0170(&g_addhero_entries[page][g_addhero_entry_state]);
        g_addhero_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 addhero_scan_next_entry(s32 page)
{
    s32 entry_index;
    s32 used_blocks;
    s32 entry_offset;
    s32 selected_entry;
    s32 entry_count;
    s32 card_full;

    if (nextfile(&g_addhero_entries[page][g_addhero_entry_state]) != 0)
    {
        func_800B0170(&g_addhero_entries[page][g_addhero_entry_state]);
        g_addhero_entry_state += 1;
        return 1;
    }

    func_800AA02C();
    if ((g_addhero_mode == 0) && (addhero_has_known_entry_type() == 0))
    {
        g_addhero_entry_state = 0xF8;
    }
    else
    {
        entry_index = 0;
        used_blocks = 0;
        g_addhero_has_free_entry_space = 0;
        entry_count = g_addhero_entry_state;
        if (entry_count > 0)
        {
            u8* entries;
            do
            {
                entries = (u8*)g_addhero_entries;
            } while (0);
            entry_offset = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
            do
            {
                used_blocks += ((struct DIRENTRY*)(entry_offset + (s32)entries))->size / ADDHERO_CARD_BLOCK_BYTES;
                entry_index++;
                entry_offset += ADDHERO_DIRECTORY_ENTRY_BYTES;
            } while (entry_index < entry_count);
        }
        card_full = used_blocks >= ADDHERO_USED_BLOCK_LIMIT;
        if (card_full != 0)
        {
            selected_entry = addhero_rank_entries(used_blocks, entry_index, entry_count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_entry_state = 0xFA;
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected_entry;
                addhero_scroll_to_selection();
            }
        }
        else
        {
            g_addhero_has_free_entry_space = 1;
            selected_entry = addhero_rank_entries(used_blocks, entry_index, entry_count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_selected_row = 0;
                addhero_scroll_to_selection();
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected_entry;
                addhero_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Prepare the currently selected directory entry for loading: set the
 *        selection status, build its file spec, and arm the read step.
 * @see decomp.me (100.00%)
 */
void addhero_commit_selected_entry(void)
{
    AddheroSelectedFilePath path;
    u8* path_text;
    s32 slot;
    s32 slot_character;

    if (g_addhero_entry_state == 0)
    {
        g_addhero_selection_status = 3;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_addhero_selection_status = 2;
        return;
    }
    memcpy(&path, &g_addhero_file_template, 6);
    path_text = (u8*)&path;
    strcat(path_text, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name);

    slot_character = path.device.characters.slot;
    slot = (u8)g_addhero_card_slot;
    g_addhero_selection_status = 0;
    slot_character += slot;
    path.device.characters.slot = slot_character;
    strcpy(g_addhero_save_file_path, path_text, slot);

    g_addhero_load_step = g_addhero_loadseq_file_ready;
    if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_addhero_selected_entry_extended = 1;
    }
    else
    {
        g_addhero_selected_entry_extended = 0;
    }
    g_addhero_io_busy = 1;
}

/**
 * @brief Consume pending software memory-card events.
 * @see decomp.me (100.00%)
 */
void addhero_clear_software_card_events(void)
{
    TestEvent(g_addhero_software_event_io_complete);
    TestEvent(g_addhero_software_event_error);
    TestEvent(g_addhero_software_event_timeout);
    TestEvent(g_addhero_software_event_new_card);
}

/**
 * @brief Consume pending hardware memory-card events.
 * @see decomp.me (100.00%)
 */
void addhero_clear_hardware_card_events(void)
{
    TestEvent(g_addhero_hardware_event_io_complete);
    TestEvent(g_addhero_hardware_event_error);
    TestEvent(g_addhero_hardware_event_timeout);
    TestEvent(g_addhero_hardware_event_new_card);
}

/**
 * @brief Consume the first pending software memory-card event.
 * @return Event index: 0 complete, 1 error, 2 timeout, 3 new card; -1 if none.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_software_card_events(void)
{
    if (TestEvent(g_addhero_software_event_io_complete) == 1)
    {
        return 0;
    }
    if (TestEvent(g_addhero_software_event_error) == 1)
    {
        return 1;
    }
    if (TestEvent(g_addhero_software_event_timeout) == 1)
    {
        return 2;
    }
    if (TestEvent(g_addhero_software_event_new_card) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Consume the first pending hardware memory-card event.
 * @return Event index: 0 complete, 1 error, 2 timeout, 3 new card; -1 if none.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_hardware_card_events(void)
{
    if (TestEvent(g_addhero_hardware_event_io_complete) == 1)
    {
        return 0;
    }
    if (TestEvent(g_addhero_hardware_event_error) == 1)
    {
        return 1;
    }
    if (TestEvent(g_addhero_hardware_event_timeout) == 1)
    {
        return 2;
    }
    if (TestEvent(g_addhero_hardware_event_new_card) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Reorder the active card's directory entries into a stable grouping:
 *        by suffix value within each known name prefix, then a third prefix,
 *        then any remaining entries, writing the result back in place.
 * @see decomp.me (100%)
 */
void addhero_sort_entries_by_type(void)
{
    struct DIRENTRY sorted[ADDHERO_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
        {
            if (g_addhero_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy(&g_addhero_entries[g_addhero_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
        {
            if (g_addhero_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy(&g_addhero_entries[g_addhero_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            bcopy(&g_addhero_entries[g_addhero_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) != 0)
        {
            bcopy(&g_addhero_entries[g_addhero_card_slot][entry_index], &sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        bcopy(&sorted[entry_index], &g_addhero_entries[g_addhero_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}
