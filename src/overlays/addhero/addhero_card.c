#include "overlays/field/field_text.h"
#include "internal/addhero_internal.h"

/** @brief Result of a load step that needs no special handling by the caller. */
#define ADDHERO_LOAD_RESULT_PENDING 1

/** @brief Psy-Q open() mode: read access (FREAD). */
#define ADDHERO_FILE_READ 0x0001

/** @brief Psy-Q open() mode: write access (FWRITE). */
#define ADDHERO_FILE_WRITE 0x0002

/** @brief Psy-Q open() mode: create the file (FCREAT). */
#define ADDHERO_FILE_CREATE 0x0200

/** @brief Psy-Q open() mode: asynchronous I/O (FASYNC). */
#define ADDHERO_FILE_ASYNC 0x8000

/** @brief Psy-Q open() mode: number of 8 KiB blocks to allocate on create. */
#define ADDHERO_FILE_BLOCKS(count) ((count) << 16)

/** @brief Bytes read to show an entry: the card header and the first 0x100 bytes of the saved game. */
#define ADDHERO_ENTRY_READ_BYTES 0x280

/** @brief Bytes read to show an entry that is not a Legend of Mana save: its card header title and CLUT. */
#define ADDHERO_ENTRY_TITLE_READ_BYTES 0x80

/** @brief Attempts made at a synchronous card file operation before giving up. */
#define ADDHERO_FILE_OP_ATTEMPTS 20

/** @brief Retries of a failed asynchronous save read or write (ADDHERO_STEP_INIT_RETRIES). */
#define ADDHERO_SAVE_RETRIES 5

/** @brief Retries of _card_load, counted separately for errors and for a newly inserted card. */
#define ADDHERO_CARD_LOAD_RETRIES 16

/** @brief Bytes of the "bu00:" device prefix copied from g_addhero_file_template, with its terminator. */
#define ADDHERO_CARD_DEVICE_BYTES sizeof("bu00:")

/** @brief Bytes of the "bu00:*" directory pattern copied from g_addhero_entry_header_template, with its terminator. */
#define ADDHERO_CARD_PATTERN_BYTES sizeof("bu00:*")

/** @brief Card-path workspace retained while advancing a load/save sequence. */
typedef union
{
    AddheroCardDevice device;
    char text[104];
} AddheroSequenceFilePath;

/** @brief Card path used to remove a placeholder save file. */
typedef union
{
    AddheroCardDevice device;
    char text[32];
} AddheroProbeFilePath;

/** @brief Directory search pattern, including the card device and wildcard. */
typedef union
{
    AddheroCardDevice device;
    char text[16];
} AddheroDirectoryPattern;

/** @brief Buffer for the selected save file's complete card path. */
typedef union
{
    AddheroCardDevice device;
    char text[256];
} AddheroSelectedFilePath;

extern s32 g_addhero_retry_count;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_file_handle;
extern char g_addhero_target_file_path[];
extern AddheroCardPathTemplate g_addhero_entry_header_template;
extern u8 g_addhero_loadseq_file_ready[];

#include "../../common/sjis/format_decimal.inc.c"
#include "../../common/save_file/format_hex.inc.c"
#include "../../common/save_file/hex_nibble_to_ascii.inc.c"
#include "../../common/save_file/parse_hex.inc.c"
#include "../../common/save_file/parse_hex_suffix_byte.inc.c"

#include "../../common/card_directory/parse_entry_fields.inc.c"

/**
 * @brief Rank the current card's entries by parsed field value, tag "full"
 *        entries, and pick the highest-valued entry to select.
 * @return Index of the highest-valued entry.
 */
s32 addhero_rank_entries(void)
{
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;

    parse_entry_fields();
    maximum = -1;
    addhero_sort_entries_by_type();
    max_suffix = parse_entry_fields();
    addhero_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_entry_fields[g_card_slot][entry_index] >= 0)
        {
            if (g_card_entry_fields[g_card_slot][entry_index] >= maximum)
            {
                g_addhero_entry_ranks[entry_index] = next_rank;
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
                        g_addhero_entry_ranks[previous_index]++;
                    }
                }
                g_addhero_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_addhero_rank_count = next_rank;
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
    g_addhero_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix + 1;
            break;
        }
    }
    return maximum;
}

/**
 * @brief Reset the per-entry rank slots to -1 and the rank count to 0x28.
 */
void addhero_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_addhero_rank_count = 0x28;
    val = -1;
    for (i = ADDHERO_CARD_SAVE_SLOTS - 1; i >= 0; i--)
    {
        g_addhero_entry_ranks[i] = val;
    }
}

/**
 * @brief Test whether the active card holds at least one entry matching a known
 *        save-name prefix.
 * @return 1 if a known-type entry exists, 0 otherwise.
 */
s32 addhero_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0 ||
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sum the block usage of the active card's entries and test whether it
 *        has reached the card's capacity.
 * @return 1 when the used blocks reach ADDHERO_USED_BLOCK_LIMIT, 0 otherwise.
 * @note Inlined into addhero_scan_next_entry.
 */
inline s32 addhero_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        used_blocks += g_card_entries[g_card_slot][entry_index].size / ADDHERO_CARD_BLOCK_BYTES;
    }
    return used_blocks >= ADDHERO_USED_BLOCK_LIMIT;
}

/**
 * @brief Remove both placeholder save filenames from the active card.
 * @note Inlined into addhero_advance_load_sequence.
 */
inline void addhero_erase_placeholder_files(void)
{
    AddheroProbeFilePath buf;

    memcpy(&buf, &g_addhero_file_template, ADDHERO_CARD_DEVICE_BYTES);
    buf.device.characters.slot += (u8)g_card_slot;
    strcat(buf.text, g_lom_save_dummy_filename);
    erase(buf.text);

    memcpy(&buf, &g_addhero_file_template, ADDHERO_CARD_DEVICE_BYTES);
    buf.device.characters.slot += (u8)g_card_slot;
    strcat(buf.text, g_lom_pocketstation_dummy_filename);
    erase(buf.text);
}

/**
 * @brief Advance the active memory-card load/save sequence by one step.
 * @return One of the ADDHERO_LOAD_RESULT_* values describing how the caller
 *         should continue the sequence.
 * @note g_card_step walks one of the g_addhero_loadseq_* byte tables;
 *       opcodes with no case are no-ops.
 */
s32 addhero_advance_load_sequence(void)
{
    AddheroSequenceFilePath card_path;
    long card_command;
    long card_result;
    s32 result;
    s32 attempts;
    s32 poll_status;
    s32 entry_index;
    s32 empty_rank;

    memcpy(&card_path, &g_addhero_file_template, ADDHERO_CARD_DEVICE_BYTES);
    result = ADDHERO_LOAD_RESULT_PENDING;
    card_path.device.characters.slot += (u8)g_card_slot;

    if (g_card_step != NULL)
    {
        switch (*g_card_step)
        {
        case ADDHERO_STEP_CARD_INFO:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_card_slot);
            _card_info(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_CARD_INFO:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                result = ADDHERO_LOAD_RESULT_COMPLETE;
                g_addhero_selection_status = ADDHERO_SELECTION_NONE;
                g_card_entry_state = ADDHERO_ENTRY_STATE_NO_CARD;
                g_card_step++;
                break;
            case CARD_EVENT_NEW_CARD:
                g_addhero_rank_count = 0x28;
                empty_rank = -1;
                for (entry_index = ADDHERO_CARD_SAVE_SLOTS - 1; entry_index >= 0; entry_index--)
                {
                    g_addhero_entry_ranks[entry_index] = empty_rank;
                }
                g_card_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
                g_card_step = &g_addhero_loadseq_start;
                break;
            }
            break;

        case ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS:
            clear_software_card_events();
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_HARDWARE_EVENTS:
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
                result = ADDHERO_LOAD_RESULT_COMPLETE;
                g_addhero_selection_status = ADDHERO_SELECTION_NONE;
                g_card_entry_state = ADDHERO_ENTRY_STATE_NO_CARD;
                break;
            }
            break;

        case ADDHERO_STEP_CLEAR_HARDWARE_EVENTS:
            clear_hardware_card_events();
            g_card_step++;
            break;

        case ADDHERO_STEP_SCAN_ENTRIES:
            addhero_erase_placeholder_files();
            g_addhero_entry_scan_active = 1;
            if (addhero_begin_entry_scan(g_card_slot) == 0)
            {
                result = ADDHERO_LOAD_RESULT_ABORT;
                g_card_step = NULL;
                g_card_entry_state = ADDHERO_ENTRY_STATE_NO_GAME_DATA;
                g_addhero_entry_scan_active = 0;
                break;
            }
            g_card_step++;
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (addhero_scan_next_entry(g_card_slot) == 0)
                {
                    if (g_addhero_mode != 0)
                    {
                        g_addhero_selected_row = 0;
                    }
                    g_addhero_entry_scan_active = 0;
                    if (g_card_entry_state != ADDHERO_ENTRY_STATE_NO_GAME_DATA && g_card_entry_state != ADDHERO_ENTRY_STATE_CARD_FULL)
                    {
                        addhero_commit_selected_entry();
                    }
                    break;
                }
            }
            break;

        case ADDHERO_STEP_CLEAR_CARD:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_card_slot);
            _card_clear(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case ADDHERO_STEP_LOAD_CARD:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_card_slot);
            _card_load(CARD_CHANNEL(g_card_slot));
            g_addhero_primary_poll_countdown = ADDHERO_CARD_LOAD_RETRIES;
            g_addhero_secondary_poll_countdown = ADDHERO_CARD_LOAD_RETRIES;
            g_card_step++;
            break;

        case ADDHERO_STEP_DONE:
            result = ADDHERO_LOAD_RESULT_ABORT;
            g_addhero_write_in_progress = 0;
            break;

        case ADDHERO_STEP_ERASE_ENTRY:
            strcat(card_path.text, g_card_entries[g_card_slot][g_addhero_selected_row].name);
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (erase(card_path.text) != 0)
                {
                    break;
                }
            }
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_CARD_LOAD:
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
                    result = ADDHERO_LOAD_RESULT_COMPLETE;
                    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
                    g_card_entry_state = ADDHERO_ENTRY_STATE_NO_CARD;
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
                    result = ADDHERO_LOAD_RESULT_CARD_ERROR;
                    g_card_entry_state = ADDHERO_ENTRY_STATE_NO_SAVE_DATA;
                    g_card_step = g_card_steps_idle;
                }
                break;
            }
            break;

        case ADDHERO_STEP_WAIT_HARDWARE_EVENTS:
            do
            {
                poll_status = poll_hardware_card_events();
            } while (poll_status == CARD_EVENT_NONE);
            g_card_step++;
            break;

        case ADDHERO_STEP_READ_ENTRY:
            g_addhero_io_busy = 1;
            g_addhero_selection_status = ADDHERO_SELECTION_NONE;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            if (g_addhero_file_handle == -1)
            {
                break;
            }
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_entry_file,
                     g_addhero_selected_entry_extended != 0 ? ADDHERO_ENTRY_READ_BYTES : ADDHERO_ENTRY_TITLE_READ_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                break;
            }
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_ENTRY_READ:
            if (g_addhero_io_busy != 0)
            {
                poll_status = poll_software_card_events();
                if (poll_status == CARD_EVENT_COMPLETE)
                {
                    g_addhero_io_busy = 0;
                    g_addhero_selection_status = ADDHERO_SELECTION_ENTRY_READ;
                    close(g_addhero_file_handle);
                }
                else if (poll_status != CARD_EVENT_NONE)
                {
                    close(g_addhero_file_handle);
                    g_card_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
                    g_card_step = &g_addhero_loadseq_start;
                }
            }
            else
            {
                g_card_step++;
            }
            break;

        case ADDHERO_STEP_READ_SAVE:
            g_addhero_progress_active = 1;
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_status_dialog(ADDHERO_DIALOG_LOAD_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_SAVE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_addhero_progress_active = 0;
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
                    g_addhero_progress_bar_active = 0;
                    addhero_open_status_dialog(ADDHERO_DIALOG_LOAD_FAILED);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_READ_SAVE and try again. */
                g_card_step--;
                break;
            }
            break;

        case ADDHERO_STEP_CHECK_POCKETSTATION:
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (McxCardType(CARD_CHANNEL(g_card_slot)) == MCX_COMMAND_ISSUED)
                {
                    break;
                }
                VSync(0);
            }
            if (attempts != ADDHERO_FILE_OP_ATTEMPTS)
            {
                McxSync(MCX_SYNC_WAIT, &card_command, &card_result);
                if (card_result == McxErrSuccess)
                {
                    g_card_step++;
                    break;
                }
            }
            addhero_open_status_dialog(ADDHERO_DIALOG_NOT_POCKETSTATION);
            break;

        case ADDHERO_STEP_INIT_RETRIES:
            g_addhero_retry_count = ADDHERO_SAVE_RETRIES;
            g_card_step++;
            break;

        case ADDHERO_STEP_READ_BEFORE_WRITE:
            g_addhero_progress_active = 1;
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(ADDHERO_DIALOG_LOAD_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_PREWRITE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_addhero_progress_active = 0;
                g_card_step++;
                close(g_addhero_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_addhero_progress_bar_active = 0;
                    addhero_open_exit_dialog(ADDHERO_DIALOG_LOAD_FAILED);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_READ_BEFORE_WRITE and try again. */
                close(g_addhero_file_handle);
                g_card_step--;
                break;
            }
            break;

        case ADDHERO_STEP_WRITE_SAVE:
            if (g_addhero_has_free_entry_space == 0)
            {
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_addhero_save_file_path) != 0)
                    {
                        break;
                    }
                }
            }
            strcat(card_path.text, g_lom_save_dummy_filename);
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(card_path.text, ADDHERO_FILE_CREATE | ADDHERO_FILE_BLOCKS(2));
            if (g_addhero_file_handle == -1)
            {
                close(-1);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(card_path.text) != 0)
                    {
                        break;
                    }
                }
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(ADDHERO_DIALOG_SAVE_FAILED);
                    return result;
                }
                break;
            }
            close(g_addhero_file_handle);
            strcpy(g_addhero_target_file_path, card_path.text);
            _card_wait(g_card_slot);
            g_addhero_file_handle = open(g_addhero_target_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_WRITE);
            clear_software_card_events();
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_card_slot);
            if (write(g_addhero_file_handle, &g_addhero_save_file, SAVE_FILE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_addhero_target_file_path) != 0)
                    {
                        break;
                    }
                }
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(ADDHERO_DIALOG_SAVE_FAILED);
                    return result;
                }
                break;
            }
            g_card_step++;
            break;

        case ADDHERO_STEP_POLL_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                if (g_addhero_has_free_entry_space != 0)
                {
                    _card_wait(g_card_slot);
                    for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_addhero_save_file_path) != 0)
                        {
                            break;
                        }
                    }
                }
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_addhero_target_file_path, g_addhero_save_file_path) != 0)
                    {
                        break;
                    }
                }
                g_addhero_write_in_progress = 0;
                g_card_step++;
                close(g_addhero_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_addhero_progress_bar_active = 0;
                    addhero_open_exit_dialog(ADDHERO_DIALOG_SAVE_FAILED);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_WRITE_SAVE and try again. */
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

/**
 * @brief Register and enable software and hardware memory-card events, then
 *        clear the progress and scan flags.
 */
void addhero_init_card_events(void)
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
    g_addhero_progress_bar_active = 0;
    g_addhero_entry_scan_active = 0;
}

#include "../../common/card_events/shutdown_card_events.inc.c"

/**
 * @brief Reset browser state and read the first directory entry of the given
 *        card page, priming the scan.
 * @param page Card page index to begin scanning.
 * @return 1 if a first entry was read, 0 if the page is empty.
 */
s32 addhero_begin_entry_scan(s32 page)
{
    AddheroDirectoryPattern buf;

    memcpy(&buf, &g_addhero_entry_header_template, ADDHERO_CARD_PATTERN_BYTES);
    g_addhero_selected_row = 0;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_card_entry_state = 0;
    buf.device.characters.slot += page;
    if (firstfile(buf.text, &g_card_entries[page][0]) != 0)
    {
        field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
        g_card_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
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
    if ((g_addhero_mode == 0) && (addhero_has_known_entry_type() == 0))
    {
        g_card_entry_state = ADDHERO_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        g_addhero_has_free_entry_space = 0;
        if (addhero_entry_blocks_reach_limit())
        {
            selected_entry = addhero_rank_entries();
            if (addhero_has_known_entry_type() == 0)
            {
                g_card_entry_state = ADDHERO_ENTRY_STATE_CARD_FULL;
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
            selected_entry = addhero_rank_entries();
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
 */
void addhero_commit_selected_entry(void)
{
    AddheroSelectedFilePath path;

    if (g_card_entry_state == 0)
    {
        g_addhero_selection_status = ADDHERO_SELECTION_EMPTY_CARD;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_addhero_selected_row].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_addhero_selection_status = ADDHERO_SELECTION_NEW_SAVE;
        return;
    }
    memcpy(&path, &g_addhero_file_template, ADDHERO_CARD_DEVICE_BYTES);
    strcat(path.text, g_card_entries[g_card_slot][g_addhero_selected_row].name);
    path.device.characters.slot += (u8)g_card_slot;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
    strcpy(g_addhero_save_file_path, path.text);
    g_card_step = g_addhero_loadseq_file_ready;
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_addhero_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_addhero_selected_entry_extended = 1;
    }
    else
    {
        g_addhero_selected_entry_extended = 0;
    }
    g_addhero_io_busy = 1;
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

/**
 * @brief Reorder the active card's directory entries into a stable grouping:
 *        by suffix value within each known name prefix, then a third prefix,
 *        then any remaining entries, writing the result back in place.
 */
void addhero_sort_entries_by_type(void)
{
    struct DIRENTRY sorted[CARD_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < 8; suffix++)
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

    for (suffix = 0; suffix < 8; suffix++)
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
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) != 0)
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
