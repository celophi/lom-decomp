#include "internal/carda_card.h"
#include "internal/carda_internal.h"
#include <fcntl.h>

/** @brief Size of the PocketStation (Ring Ring Land) save file: six memory-card blocks. */
#define CARDA_POCKETSTATION_SAVE_BYTES 0xC000

/** @brief Used-block count that leaves no room for the Ring Ring Land file. */
#define CARDA_POCKETSTATION_USED_BLOCK_LIMIT (CARD_SAVE_SLOTS + 1 - CARDA_POCKETSTATION_SAVE_BYTES / CARD_BLOCK_BYTES)

/** @brief Frames (two seconds) to wait for McxCardType to accept its command. */
#define CARDA_CARD_TYPE_WAIT_FRAMES 120

/** @brief Rank slots carda_reset_entry_ranks clears, two more than CARD_SAVE_SLOTS. */
#define CARDA_RANK_SLOTS 17

/** @brief Suffix values whose use carda_rank_entries records. */
#define CARDA_SUFFIX_FLAG_COUNT 10

/** @brief Highest suffix the new-save entry can take; it gets the lowest unused one from 1 up. */
#define CARDA_NEW_SAVE_SUFFIX_MAX 8

/** @brief Leading part of a save read back by CARDA_STEP_READ_SAVE_PREFIX. */
#define CARDA_SAVE_PREFIX_BYTES 0x400

/** @brief Offset of the title's note sign (CARDA_TITLE_NOTE_INDEX) in a card file's first sector. */
#define CARDA_TITLE_NOTE_OFFSET (CARD_FILE_TITLE_OFFSET + CARDA_TITLE_NOTE_INDEX)

static s32 carda_rank_entries(void);
static s32 carda_has_known_entry_type(void);
s32 carda_card_lacks_free_blocks(void);
void carda_erase_placeholder_files(void);
static s32 carda_begin_entry_scan(s32 page);
static s32 carda_scan_next_entry(s32 page);
static void carda_sort_entries_by_type(void);

#include "../../common/sjis/format_decimal.inc.c"
#include "../../common/save_file/format_hex.inc.c"
#include "../../common/save_file/hex_nibble_to_ascii.inc.c"
#include "../../common/save_file/parse_hex.inc.c"
#include "../../common/save_file/parse_hex_suffix_byte.inc.c"

#include "../../common/card_directory/parse_entry_fields.inc.c"

/**
 * @brief Sort the current card's directory, rank its saves and number the new-save entry.
 *
 * Legend of Mana saves are ranked by serial into g_card_entry_ranks, the serial a
 * new save gets goes to g_card_entry_value_limit, and the new-save placeholder gets
 * the lowest suffix (1 to 8) that no save on the card uses.
 *
 * @return Index of the save with the highest serial, or 0 when there is none.
 */
static s32 carda_rank_entries(void)
{
    s32 suffix_used[CARDA_SUFFIX_FLAG_COUNT];
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;
    u8* serial_text;
    const char* filename_fields;
    s32 digits_left;
    s32 serial_value;

    parse_entry_fields();
    carda_sort_entries_by_type();
    for (entry_index = CARDA_SUFFIX_FLAG_COUNT - 1; entry_index >= 0; entry_index--)
    {
        suffix_used[entry_index] = 0;
    }

    max_suffix = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            digits_left = CARD_SERIAL_DIGITS;
            serial_text = CARD_ENTRY_SERIAL_TEXT(g_card_slot, entry_index);
            serial_value = 0;
            while ((*serial_text >= '0' && *serial_text <= '9') || (*serial_text >= 'a' && *serial_text <= 'f') || (*serial_text >= 'A' && *serial_text <= 'F'))
            {
                if (digits_left == 0)
                {
                    break;
                }
                serial_value <<= CARD_SERIAL_DIGIT_BITS;
                if (*serial_text >= '0' && *serial_text <= '9')
                {
                    u32 adjusted_serial;

                    adjusted_serial = serial_value - '0';
                    serial_value = adjusted_serial + *serial_text;
                }
                else if (*serial_text >= 'A' && *serial_text <= 'F')
                {
                    u32 adjusted_serial;

                    adjusted_serial = serial_value - ('A' - CARD_SERIAL_DECIMAL_DIGITS);
                    serial_value = adjusted_serial + *serial_text;
                }
                else if (*serial_text >= 'a' && *serial_text <= 'f')
                {
                    u32 adjusted_serial;

                    adjusted_serial = serial_value - ('a' - CARD_SERIAL_DECIMAL_DIGITS);
                    serial_value = adjusted_serial + *serial_text;
                }
                serial_text++;
                digits_left--;
            }
            filename_fields = &g_card_entries[g_card_slot][entry_index].name[CARD_SAVE_FILENAME_PREFIX_LENGTH];
            g_card_entry_fields[g_card_slot][entry_index] = serial_value;
            g_card_entry_suffix_values[entry_index] = parse_hex_suffix_byte(filename_fields);
            suffix_used[g_card_entry_suffix_values[entry_index]] = 1;
            if (max_suffix < g_card_entry_suffix_values[entry_index])
            {
                max_suffix = g_card_entry_suffix_values[entry_index];
            }
        }
        else
        {
            g_card_entry_fields[g_card_slot][entry_index] = CARD_ENTRY_SERIAL_NONE;
            g_card_entry_suffix_values[entry_index] = 0;
        }
    }

    maximum = -1;
    carda_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_entry_fields[g_card_slot][entry_index] >= 0)
        {
            if (g_card_entry_fields[g_card_slot][entry_index] >= maximum)
            {
                g_card_entry_ranks[entry_index] = next_rank;
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
                        g_card_entry_ranks[previous_index]++;
                    }
                }
                g_card_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_card_rank_count = next_rank;
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
    g_card_entry_value_limit = next_rank + 1;

    /* Reuse max_suffix as the lowest suffix no recognized entry uses. */
    for (max_suffix = 1; max_suffix <= CARDA_NEW_SAVE_SUFFIX_MAX; max_suffix++)
    {
        if (suffix_used[max_suffix] == 0)
        {
            break;
        }
    }
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix;
            break;
        }
    }
    return maximum;
}

/** @brief Mark every rank slot unused and reset the rank count. */
inline void carda_reset_entry_ranks(void)
{
    s32 rank_index;
    s32 empty_rank;

    g_card_rank_count = CARD_RANK_COUNT_NONE;
    empty_rank = -1;
    for (rank_index = CARDA_RANK_SLOTS - 1; rank_index >= 0; rank_index--)
    {
        g_card_entry_ranks[rank_index] = empty_rank;
    }
}

/**
 * @brief Check whether the current card holds a file this mode can use.
 * @return 1 if an entry is a Legend of Mana save or a Ring Ring Land file (only
 *         Ring Ring Land when sending a pet), otherwise 0.
 */
static s32 carda_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_menu_mode != CARDA_MODE_SEND_PET)
        {
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0 ||
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                return 1;
            }
        }
        if (g_card_menu_mode == CARDA_MODE_SEND_PET &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Check whether the current card lacks room for a new file.
 * @return 1 if the used blocks leave no room for the mode's file, otherwise 0.
 * @note Sending a pet writes the six-block Ring Ring Land file; every other mode
 *       writes a two-block save.
 */
inline s32 carda_card_lacks_free_blocks(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        used_blocks += g_card_entries[g_card_slot][entry_index].size / CARD_BLOCK_BYTES;
    }
    if ((used_blocks < CARD_USED_BLOCK_LIMIT && g_card_menu_mode != CARDA_MODE_SEND_PET) ||
        (used_blocks < CARDA_POCKETSTATION_USED_BLOCK_LIMIT && g_card_menu_mode == CARDA_MODE_SEND_PET))
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Erase the save and Ring Ring Land placeholder files left by an interrupted
 *        write, then clear pending card events.
 */
inline void carda_erase_placeholder_files(void)
{
    CardFilePath card_path;

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += g_card_slot;
    strcat(card_path.text, g_lom_save_dummy_filename);
    erase(card_path.text);

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += g_card_slot;
    strcat(card_path.text, g_lom_pocketstation_dummy_filename);
    erase(card_path.text);
    clear_software_card_events();
    clear_hardware_card_events();
}

/**
 * @brief Run the current step of the card sequence and advance g_card_step.
 * @return CardMenuSequenceResult for the caller.
 * @note g_card_step walks one of the g_card_steps_* or g_carda_steps_* byte tables.
 *       Each CardMenuStep or CardaCardStep opcode issues or polls a card command,
 *       reads or writes a save, or scans the card directory. New files are written
 *       under a placeholder name first and renamed once the write has finished.
 *       Opcodes without a case make the sequence wait on them.
 */
s32 carda_advance_card_sequence(void)
{
    CardFilePath card_path;
    CardFilePath file_name;
    CardFilePath device_path;
    u8 title_frame[CARD_SECTOR_BYTES];
    struct DIRENTRY dir_entry;
    s32 attempts;
    s32 check_attempts;
    s32 poll_result;
    s32 phase_result;
    long command;
    long result;
    u8* title_src;
    u8* title_dst;

    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    card_path.device.characters.slot += g_card_slot;
    strcpy(device_path.text, card_path.text);
    phase_result = CARD_MENU_SEQUENCE_WAIT;
    if (CARDA_IS_POCKETSTATION_MODE(g_card_menu_mode))
    {
        if (g_card_step == g_card_steps_initial_scan)
        {
            g_card_step = g_carda_steps_initial_scan_check_type;
        }
    }
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
            poll_result = poll_software_card_events();
            switch (poll_result)
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
                carda_reset_entry_ranks();
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_card_steps_initial_scan;
                break;
            }
            break;

        case CARDA_STEP_POLL_CARD_PRESENT:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                carda_open_status_dialog(CARDA_DIALOG_FORMAT_FAILED);
                break;
            }
            break;

        case CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS:
            clear_software_card_events();
            g_card_step++;
            break;

        case CARD_MENU_STEP_POLL_HARDWARE_EVENTS:
            while ((poll_result = poll_hardware_card_events()) == CARD_EVENT_NONE);
            switch (poll_result)
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                phase_result = CARD_MENU_SEQUENCE_NO_CARD;
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
            carda_erase_placeholder_files();
            g_card_menu_entry_scan_active = 1;
            if (carda_begin_entry_scan(g_card_slot) == 0)
            {
                phase_result = CARD_MENU_SEQUENCE_FINISHED;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                g_card_step = NULL;
                g_card_menu_entry_scan_active = 0;
                break;
            }
            g_card_step++;
            for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
            {
                if (carda_scan_next_entry(g_card_slot) == 0)
                {
                    g_card_menu_entry_scan_active = 0;
                    if (g_card_entry_state != CARD_MENU_ENTRY_STATE_NO_GAME_DATA && g_card_entry_state != CARD_MENU_ENTRY_STATE_CARD_FULL)
                    {
                        if (g_card_entry_state != CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD)
                        {
                            carda_commit_selected_entry();
                        }
                    }
                    break;
                }
            }
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

        case CARDA_STEP_SKIP:
            g_card_step++;
            break;

        case CARDA_STEP_CREATE_TEMP_SAVE:
            if (g_card_menu_has_free_space == 0)
            {
                strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
                CARD_ERASE_WITH_RETRIES(attempts, card_path.text);
                clear_software_card_events();
                clear_hardware_card_events();
                strcpy(card_path.text, device_path.text);
            }
            strcpy(file_name.text, g_lom_save_dummy_filename);
            strcat(card_path.text, file_name.text);
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(card_path.text, FCREAT | CARD_FILE_BLOCKS(SAVE_FILE_BYTES / CARD_BLOCK_BYTES));
            if (g_card_menu_file_handle == CARD_FILE_ERROR)
            {
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count == 0)
                {
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_card_menu_file_handle);
            strcpy(g_card_temp_save_path, card_path.text);
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_SAVE:
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_temp_save_path, FASYNC | FWRITE);
            clear_software_card_events();
            g_card_menu_progress_bar_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            _card_wait(g_card_slot);
            if (write(g_card_menu_file_handle, g_carda_save_blob, SAVE_FILE_BYTES) == CARD_FILE_ERROR)
            {
                close(g_card_menu_file_handle);
                CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_TEMP_SAVE and try again. */
                    g_card_step--;
                    break;
                }
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_TEMP_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                if (g_card_menu_has_free_space != 0)
                {
                    strcpy(card_path.text, device_path.text);
                    strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
                    _card_wait(g_card_slot);
                    CARD_ERASE_WITH_RETRIES(attempts, card_path.text);
                    clear_software_card_events();
                    clear_hardware_card_events();
                }

                /* Final name: prefix, serial in hex without leading zeros, then +/- and the suffix digit. */
                strcpy(card_path.text, device_path.text);
                strcpy(file_name.text, g_lom_save_filename_prefix);
                format_hex(&file_name.text[CARD_SAVE_FILENAME_PREFIX_LENGTH], g_card_entry_value_limit, CARD_SERIAL_DIGITS);
                strcat(card_path.text, file_name.text);
                if (carda_test_option_flag_2() != 0)
                {
                    file_name.text[0] = '+';
                }
                else
                {
                    file_name.text[0] = '-';
                }
                file_name.text[1] = g_card_entry_suffix_values[g_card_menu_selected_row] + '0';
                file_name.text[2] = 0;
                strcat(card_path.text, file_name.text);

                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_card_temp_save_path, card_path.text) != 0)
                    {
                        break;
                    }
                }

                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (firstfile(card_path.text, &dir_entry) != 0)
                    {
                        break;
                    }
                }
                if (attempts != CARD_MENU_FILE_OP_ATTEMPTS)
                {
                    /* Replace the title in the file's first sector. */
                    _card_wait(g_card_slot);
                    _card_read(CARD_CHANNEL(g_card_slot), dir_entry.head, title_frame);
                    _card_wait(g_card_slot);
                    title_dst = &title_frame[CARD_FILE_TITLE_OFFSET];
                    title_src = g_carda_save_title_template;
                    attempts = 0;
                    while (attempts < CARDA_TITLE_TEMPLATE_BYTES)
                    {
                        attempts++;
                        *title_dst++ = *title_src++;
                    }
                    if (carda_test_option_flag_2() != 0)
                    {
                        title_frame[CARDA_TITLE_NOTE_OFFSET] = SJIS_NOTE_SIGN >> SJIS_CODE_LEAD_SHIFT;
                        title_frame[CARDA_TITLE_NOTE_OFFSET + 1] = SJIS_NOTE_SIGN & SJIS_CODE_TRAIL_MASK;
                    }
                    _card_wait(g_card_slot);
                    _card_write(CARD_CHANNEL(g_card_slot), dir_entry.head, title_frame);
                    _card_wait(g_card_slot);
                }
                g_card_menu_write_in_progress = 0;
                g_card_step++;
                close(g_card_menu_file_handle);
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_card_steps_initial_scan;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_TEMP_SAVE and try again. */
                    close(g_card_menu_file_handle);
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                    g_card_step -= 2;
                }
                else
                {
                    close(g_card_menu_file_handle);
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                    g_card_menu_progress_bar_active = 0;
                    g_card_menu_progress_start_tick = VSync(-1);
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            break;

        case CARDA_STEP_CREATE_POCKETSTATION_SAVE:
            g_card_menu_progress_bar_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) != 0)
            {
                strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
                CARD_ERASE_WITH_RETRIES(attempts, card_path.text);
                clear_software_card_events();
                clear_hardware_card_events();
            }
            strcpy(card_path.text, device_path.text);
            strcat(card_path.text, g_lom_pocketstation_filename_prefix);
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(card_path.text, FCREAT | CARD_FILE_BLOCKS(CARDA_POCKETSTATION_SAVE_BYTES / CARD_BLOCK_BYTES));
            if (g_card_menu_file_handle == CARD_FILE_ERROR)
            {
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count == 0)
                {
                    close(g_card_menu_file_handle);
                    carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_card_menu_file_handle);
            strcpy(g_card_temp_save_path, card_path.text);
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_POCKETSTATION_SAVE:
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_temp_save_path, FASYNC | FWRITE);
            _card_wait(g_card_slot);
            if (write(g_card_menu_file_handle, g_carda_save_blob, CARDA_POCKETSTATION_SAVE_BYTES) == CARD_FILE_ERROR)
            {
                McxHideTrans(CARD_CHANNEL(g_card_slot));
                close(g_card_menu_file_handle);
                CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_POCKETSTATION_SAVE and try again. */
                    g_card_step--;
                    break;
                }
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_POCKETSTATION_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_menu_write_in_progress = 0;
                g_card_step++;
                close(g_card_menu_file_handle);
                g_card_step = g_card_steps_initial_scan;
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (firstfile(g_card_temp_save_path, &dir_entry) != 0)
                    {
                        break;
                    }
                }
                if (attempts == CARD_MENU_FILE_OP_ATTEMPTS)
                {
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (McxExecFlag(CARD_CHANNEL(g_card_slot), dir_entry.head / CARD_BLOCK_SECTORS, 1) != 0)
                    {
                        break;
                    }
                }
                if (attempts != CARD_MENU_FILE_OP_ATTEMPTS)
                {
                    return phase_result;
                }
                CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                return phase_result;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_POCKETSTATION_SAVE and try again. */
                    close(g_card_menu_file_handle);
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                    g_card_step -= 2;
                }
                else
                {
                    close(g_card_menu_file_handle);
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    g_card_menu_progress_bar_active = 0;
                    g_card_menu_progress_start_tick = VSync(-1);
                }
                break;
            }
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
                if (g_card_menu_load_error_retries != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    clear_software_card_events();
                    _card_load(CARD_CHANNEL(g_card_slot));
                    break;
                }
                phase_result = CARD_MENU_SEQUENCE_NO_CARD;
                g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                break;
            case CARD_EVENT_NEW_CARD:
                g_card_menu_new_card_retries--;
                if (g_card_menu_new_card_retries != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    clear_software_card_events();
                    _card_load(CARD_CHANNEL(g_card_slot));
                    break;
                }
                phase_result = CARD_MENU_SEQUENCE_UNFORMATTED;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_SAVE_DATA;
                g_card_step = g_card_steps_idle;
                break;
            }
            break;

        case CARD_MENU_STEP_WAIT_HARDWARE_EVENTS:
            _card_wait(g_card_slot);
            g_card_step++;
            break;

        case CARD_MENU_STEP_READ_ENTRY:
            g_card_menu_io_busy = 1;
            g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            if (g_card_menu_file_handle != CARD_FILE_ERROR)
            {
                clear_software_card_events();
                _card_wait(g_card_slot);
                if (read(g_card_menu_file_handle, &g_carda_selected_file,
                         g_card_menu_selected_entry_extended != 0 ? CARD_MENU_ENTRY_READ_BYTES : CARD_MENU_ENTRY_TITLE_READ_BYTES) == CARD_FILE_ERROR)
                {
                    close(g_card_menu_file_handle);
                    return phase_result;
                }
                g_card_step++;
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
            else if (poll_result != CARD_EVENT_NONE)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                g_card_menu_io_busy = 0;
                close(g_card_menu_file_handle);
                g_card_step = g_card_steps_initial_scan;
            }
            break;

        case CARD_MENU_STEP_READ_SAVE:
            g_card_menu_progress_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_card_menu_file_handle, g_carda_save_blob, SAVE_FILE_BYTES) == CARD_FILE_ERROR)
            {
                carda_open_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            g_card_step++;
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
                g_card_menu_progress_start_tick = VSync(-1);
                g_card_menu_progress_bar_active = 0;
                carda_open_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                break;
            }
            break;

        case CARD_MENU_STEP_CHECK_POCKETSTATION:
            for (check_attempts = 0; check_attempts < CARD_MENU_FILE_OP_ATTEMPTS; check_attempts++)
            {
                for (attempts = 0; attempts < CARDA_CARD_TYPE_WAIT_FRAMES; attempts++)
                {
                    if (McxCardType(CARD_CHANNEL(g_card_slot)) == MCX_COMMAND_ISSUED)
                    {
                        break;
                    }
                    VSync(0);
                }
                VSync(0);
                if (attempts != CARD_MENU_FILE_OP_ATTEMPTS)
                {
                    McxSync(MCX_SYNC_WAIT, &command, &result);
                    switch (result)
                    {
                    case McxErrSuccess:
                        g_card_step++;
                        return phase_result;
                    case McxErrNoCard:
                        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_CARD;
                        g_card_step = NULL;
                        return phase_result;
                    }
                }
            }
            if (g_card_entry_state != CARDA_ENTRY_STATE_NOT_POCKETSTATION)
            {
                g_card_entry_state = CARDA_ENTRY_STATE_NOT_POCKETSTATION;
            }
            g_card_step = NULL;
            return phase_result;

        case CARDA_STEP_READ_SAVE_PREFIX:
            g_card_menu_progress_active = 1;
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_selected_save_path, FASYNC | FREAD);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_card_menu_file_handle, g_carda_save_blob, CARDA_SAVE_PREFIX_BYTES) == CARD_FILE_ERROR)
            {
                carda_open_save_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_SAVE_PREFIX_READ:
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
                g_card_menu_progress_bar_active = 0;
                carda_open_save_status_dialog(CARD_MENU_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            break;

        case CARD_MENU_STEP_INIT_RETRIES:
            g_card_menu_retry_count = CARD_MENU_SAVE_RETRIES;
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_POCKETSTATION_SAVE:
            if (g_card_menu_has_free_space == 0)
            {
                CARD_ERASE_WITH_RETRIES(attempts, g_card_selected_save_path);
            }
            strcat(card_path.text, g_lom_pocketstation_dummy_filename);
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(card_path.text, FCREAT | CARD_FILE_BLOCKS(CARDA_POCKETSTATION_SAVE_BYTES / CARD_BLOCK_BYTES));
            if (g_card_menu_file_handle == CARD_FILE_ERROR)
            {
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count == 0)
                {
                    close(g_card_menu_file_handle);
                    CARD_ERASE_WITH_RETRIES(attempts, card_path.text);
                    carda_open_save_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_card_menu_file_handle);
            strcpy(g_card_temp_save_path, card_path.text);
            _card_wait(g_card_slot);
            g_card_menu_file_handle = open(g_card_temp_save_path, FASYNC | FWRITE);
            clear_software_card_events();
            g_card_menu_progress_start_tick = VSync(-1);
            g_card_menu_progress_bar_active = 1;
            _card_wait(g_card_slot);
            if (write(g_card_menu_file_handle, g_carda_save_blob, CARDA_POCKETSTATION_SAVE_BYTES) == CARD_FILE_ERROR)
            {
                close(g_card_menu_file_handle);
                CARD_ERASE_WITH_RETRIES(attempts, g_card_temp_save_path);
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count == 0)
                {
                    carda_open_save_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_TEMP_POCKETSTATION_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                if (g_card_menu_has_free_space != 0)
                {
                    _card_wait(g_card_slot);
                    CARD_ERASE_WITH_RETRIES(attempts, g_card_selected_save_path);
                }
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARD_MENU_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_card_temp_save_path, g_card_selected_save_path) != 0)
                    {
                        break;
                    }
                }
                g_card_menu_write_in_progress = 0;
                g_card_step++;
                close(g_card_menu_file_handle);
                g_card_step = g_card_steps_initial_scan;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_card_menu_retry_count--;
                if (g_card_menu_retry_count == 0)
                {
                    close(g_card_menu_file_handle);
                    g_card_menu_progress_bar_active = 0;
                    carda_open_save_status_dialog(CARD_MENU_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                /* Rewind to CARDA_STEP_WRITE_TEMP_POCKETSTATION_SAVE and try again. */
                close(g_card_menu_file_handle);
                g_card_step--;
                break;
            }
            break;
        }
    }
    return phase_result;
}

/**
 * @brief Format the current card and reset the list to a single new-save entry.
 */
void carda_format_card(void)
{
    s32 unused[4];
    s32 attempt;

    for (attempt = 0; attempt < CARD_MENU_FILE_OP_ATTEMPTS; attempt++)
    {
        if (_card_format(CARD_CHANNEL(g_card_slot)) != 0)
        {
            break;
        }
    }

    g_card_menu_has_free_space = 0;
    g_card_menu_selected_row = 0;
    strcpy(g_card_entries[g_card_slot][0].name, g_new_save_entry_prefix);
}

#include "../../common/card_events/restart_card_sequence.inc.c"
#include "../../common/card_events/poll_and_retry_card_info.inc.c"

/**
 * @brief Open and enable the software and hardware memory-card events.
 */
void carda_init_card_events(void)
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
    g_card_menu_progress_start_tick = VSync(-1);
    g_card_menu_progress_bar_active = 0;
    g_card_menu_entry_scan_active = 0;
}

#include "../../common/card_events/shutdown_card_events.inc.c"

/**
 * @brief Reset the list view and read the first directory entry of a card.
 * @param page Memory-card slot to scan.
 * @return 1 if the first entry was read or the mode allows an empty card, 0 otherwise.
 */
static s32 carda_begin_entry_scan(s32 page)
{
    CardSearchPattern search_path;
    s32 attempt;

    strcpy(search_path.text, CARD_SEARCH_PATTERN);
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    search_path.device.characters.slot += page;
    _card_wait(page);
    g_card_entry_state = 0;
    for (attempt = 0; attempt < CARD_MENU_FILE_OP_ATTEMPTS; attempt++)
    {
        if (firstfile(search_path.text, g_card_entries[page]) != 0)
        {
            field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
            g_card_entry_state += 1;
            return 1;
        }
    }
    if (g_card_menu_mode == CARDA_MODE_LOAD || g_card_menu_mode == CARDA_MODE_RETURN_PET)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Read the next directory entry, or finish and rank the completed directory.
 * @param page Memory-card slot being scanned.
 * @return 1 if another directory entry was read, otherwise 0.
 * @note When the directory is complete, a placeholder entry is appended (full card
 *       or new save, by mode), the entries are ranked and the selection is set.
 */
static s32 carda_scan_next_entry(s32 page)
{
    s32 entry_attempt;
    s32 selected;

    _card_wait(page);
    for (entry_attempt = 0; entry_attempt < CARD_MENU_FILE_OP_ATTEMPTS; entry_attempt++)
    {
        if (nextfile(&g_card_entries[page][g_card_entry_state]) != 0)
        {
            field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
            g_card_entry_state += 1;
            return 1;
        }
    }

    field_reset_input_repeat();
    if (g_card_menu_mode == CARDA_MODE_LOAD && carda_has_known_entry_type() == 0)
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        g_card_menu_has_free_space = 0;
        if (carda_card_lacks_free_blocks())
        {
            if (g_card_menu_mode == CARDA_MODE_SAVE || g_card_menu_mode == CARDA_MODE_SEND_PET)
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_card_full_entry_name);
                g_card_entries[page][g_card_entry_state].size = 0;
                g_card_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
            {
                if (CARDA_IS_POCKETSTATION_MODE(g_card_menu_mode))
                {
                    g_card_entry_state = CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD;
                }
                else
                {
                    g_card_entry_state = CARD_MENU_ENTRY_STATE_CARD_FULL;
                }
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
            if (CARDA_IS_POCKETSTATION_MODE(g_card_menu_mode))
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_new_save_entry_prefix);
                g_card_entries[page][g_card_entry_state].size = CARDA_POCKETSTATION_SAVE_BYTES;
                g_card_entry_state += 1;
            }
            else if (g_card_menu_mode == CARDA_MODE_SAVE)
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_new_save_entry_prefix);
                g_card_entries[page][g_card_entry_state].size = SAVE_FILE_BYTES;
                g_card_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
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

/**
 * @brief Commit the selected entry and start reading its file header.
 * @note An empty card, the new-save placeholder and the full-card placeholder only
 *       set g_card_menu_selection_status. Any other entry gets its path built into
 *       g_card_selected_save_path and the g_card_steps_read_selected_header table armed.
 */
void carda_commit_selected_entry(void)
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
    if (strncmp(g_card_full_entry_name, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) == 0)
    {
        g_card_menu_selection_status = CARDA_SELECTION_CARD_FULL;
        return;
    }
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
    card_path.device.characters.slot += g_card_slot;
    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
    strcpy(g_card_selected_save_path, card_path.text);
    g_card_step = g_card_steps_read_selected_header;
    g_card_menu_io_busy = 1;
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_card_menu_selected_entry_extended = 1;
    }
    else
    {
        g_card_menu_selected_entry_extended = 0;
    }
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

/**
 * @brief Reorder the current card's directory by save type and suffix.
 * @note Legend of Mana saves come first, then Ring Ring Land files (each grouped by
 *       suffix), then the new-save and full-card placeholders, then every other file.
 */
static void carda_sort_entries_by_type(void)
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
                strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted_entries[output_count], sizeof(struct DIRENTRY));
                output_count++;
            }
        }
    }

    for (group = 0; group < CARD_MENU_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == group &&
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted_entries[output_count], sizeof(struct DIRENTRY));
                output_count++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0 ||
            strncmp(g_card_full_entry_name, g_card_entries[g_card_slot][entry_index].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) == 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted_entries[output_count], sizeof(struct DIRENTRY));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) != 0 &&
            strncmp(g_card_full_entry_name, g_card_entries[g_card_slot][entry_index].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) != 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted_entries[output_count], sizeof(struct DIRENTRY));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        bcopy((u8*)&sorted_entries[entry_index], (u8*)&g_card_entries[g_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}
