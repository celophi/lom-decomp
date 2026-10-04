#include "internal/carda_card.h"
#include "internal/carda_internal.h"
#include "sdk/kernel.h"

/** @brief Psy-Q open() mode: read access (FREAD). */
#define CARDA_FILE_READ 0x0001

/** @brief Psy-Q open() mode: write access (FWRITE). */
#define CARDA_FILE_WRITE 0x0002

/** @brief Psy-Q open() mode: create the file (FCREAT). */
#define CARDA_FILE_CREATE 0x0200

/** @brief Psy-Q open() mode: asynchronous I/O (FASYNC). */
#define CARDA_FILE_ASYNC 0x8000

/** @brief Psy-Q open() mode: number of 8 KiB blocks to allocate on create. */
#define CARDA_FILE_BLOCKS(count) ((count) << 16)

/** @brief Size of the main save blob (two memory-card blocks). */
#define CARDA_SAVE_BYTES 0x4000

/** @brief Size of the PocketStation (Ring Ring Land) save file: six memory-card blocks. */
#define CARDA_POCKETSTATION_SAVE_BYTES 0xC000

/** @brief Leading part of a save read back by CARDA_STEP_READ_SAVE_PREFIX. */
#define CARDA_SAVE_PREFIX_BYTES 0x400

/** @brief Number of 128-byte sectors in one memory-card block. */
#define CARDA_SECTORS_PER_BLOCK 64

/** @brief Attempts made at a synchronous card file operation before giving up. */
#define CARDA_FILE_OP_ATTEMPTS 20

/** @brief Retries of a failed asynchronous save write (CARDA_STEP_ARM_RETRIES). */
#define CARDA_SAVE_RETRIES 5

/** @brief Length of the title template (g_carda_save_title_template) copied into a save's title frame. */
#define CARDA_TITLE_TEMPLATE_BYTES 18

/** @brief Offset of the title in a memory-card title frame. */
#define CARDA_TITLE_OFFSET 4

/**
 * @brief 0x20-byte, word-aligned memory-card path scratch buffer.
 * The first six bytes are initialized from the "bu00:" device prefix before a
 * filename suffix is appended.
 */
typedef union
{
    u8 bytes[0x20];
    u32 align;
} CardaLoadScratch;

/** @brief 0x100-byte, word-aligned memory-card path buffer. */
typedef union
{
    u8 bytes[0x100];
    u32 align;
} CardaCardPathBuffer;

/** @brief 0x10-byte, word-aligned buffer initialized from the "bu00:*" search path. */
typedef union
{
    u8 bytes[0x10];
    u32 align;
} CardaCardSearchPath;

/** @brief Eight-byte, word-aligned memory-card path template ("bu00:"). */
typedef union
{
    char text[8];
    u32 align[2];
} CardaCardPathTemplate;

extern const CardaCardPathTemplate g_carda_card_path_prefix;

extern const CardaCardPathTemplate g_carda_card_search_path;

static s32 carda_rank_entries(void);
static s32 carda_has_known_entry_type(void);
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
 * @brief Sort and rank the current card's entries and number the new-save entry.
 *
 * Recognized saves are ranked by their field value into g_carda_entry_ranks, the next
 * field value is stored in g_carda_next_save_serial, and the new-save placeholder entry gets
 * the lowest suffix byte (1-8) that no recognized save uses.
 *
 * @return Index of the entry holding the greatest field value.
 */
static s32 carda_rank_entries(void)
{
    s32 suffix_used[10];
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;
    u8* cursor;
    u8* suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;

    parse_entry_fields();
    carda_sort_entries_by_type();
    for (entry_index = 9; entry_index >= 0; entry_index--)
    {
        suffix_used[entry_index] = 0;
    }

    max_suffix = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            digits_left = CARD_SERIAL_DIGITS;
            cursor = CARD_ENTRY_SERIAL_TEXT(g_card_slot, entry_index);
            value = 0;
            while (((u8)(*cursor - '0') < 10) || ((u8)(*cursor - 'a') < 6) || ((u8)(*cursor - 'A') < 6))
            {
                if (digits_left == 0)
                {
                    break;
                }
                value <<= 4;
                if ((u8)(*cursor - '0') < 10)
                {
                    decimal_base = value - '0';
                    value = decimal_base + *cursor;
                }
                else if ((u8)(*cursor - 'A') < 6)
                {
                    uppercase_base = value - ('A' - 10);
                    value = uppercase_base + *cursor;
                }
                else if ((u8)(*cursor - 'a') < 6)
                {
                    lowercase_base = value - ('a' - 10);
                    value = lowercase_base + *cursor;
                }
                cursor++;
                digits_left--;
            }
            suffix = (u8*)&g_card_entries[g_card_slot][entry_index].name[CARD_SAVE_FILENAME_PREFIX_LENGTH];
            g_card_entry_fields[g_card_slot][entry_index] = value;
            g_card_entry_suffix_values[entry_index] = parse_hex_suffix_byte(suffix);
            suffix_used[g_card_entry_suffix_values[entry_index]] = 1;
            if (max_suffix < g_card_entry_suffix_values[entry_index])
            {
                max_suffix = g_card_entry_suffix_values[entry_index];
            }
        }
        else
        {
            g_card_entry_fields[g_card_slot][entry_index] = -1;
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
                g_carda_entry_ranks[entry_index] = next_rank;
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
                        g_carda_entry_ranks[previous_index]++;
                    }
                }
                g_carda_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_carda_rank_count = next_rank;
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
    g_carda_next_save_serial = next_rank + 1;

    /* Reuse max_suffix as the lowest suffix no recognized entry uses. */
    for (max_suffix = 1; max_suffix < 9; max_suffix++)
    {
        if (suffix_used[max_suffix] == 0)
        {
            break;
        }
    }
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix;
            break;
        }
    }
    return maximum;
}

/** @brief Mark all seventeen rank slots unused and reset the rank-count sentinel. */
void carda_reset_entry_ranks(void)
{
    s32 rank_index;
    s32 empty_rank;

    g_carda_rank_count = 0x28;
    empty_rank = -1;
    for (rank_index = 16; rank_index >= 0; rank_index--)
    {
        g_carda_entry_ranks[rank_index] = empty_rank;
    }
}

/**
 * @brief Check whether the current card holds a save this mode can use.
 * @return 1 if an entry matches g_lom_save_filename_prefix or g_lom_pocketstation_filename_prefix (only g_lom_pocketstation_filename_prefix in mode 2),
 * otherwise 0.
 */
static s32 carda_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_carda_mode != CARDA_MODE_SEND_PET)
        {
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0 ||
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                return 1;
            }
        }
        if (g_carda_mode == CARDA_MODE_SEND_PET &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Check whether the current card lacks room for a new save.
 * @return 1 if the used blocks leave no room for the mode's save file, otherwise 0.
 * @note A mode-2 save needs six blocks (fewer than 10 used), any other save two
 *       (fewer than 14 used). Inlined into carda_scan_next_entry.
 */
inline s32 carda_card_lacks_free_blocks(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        used_blocks += g_card_entries[g_card_slot][entry_index].size / CARDA_MEMORY_CARD_BLOCK_BYTES;
    }
    if ((used_blocks < 14 && g_carda_mode != CARDA_MODE_SEND_PET) || (used_blocks < 10 && g_carda_mode == CARDA_MODE_SEND_PET))
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Erase the two fixed per-slot memory-card files and clear pending card events.
 * @note Each erase starts from the six-byte "bu00:" device path, adjusts the slot
 *       digit and appends one fixed file name.
 */
inline void carda_erase_placeholder_files(void)
{
    CardaLoadScratch card_path;

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_card_slot;
    strcat(card_path.bytes, g_lom_save_dummy_filename);
    erase(&card_path);

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_card_slot;
    strcat(card_path.bytes, g_lom_pocketstation_dummy_filename);
    erase(&card_path);
    clear_software_card_events();
    clear_hardware_card_events();
}

/**
 * @brief Run the current memory-card save step and advance g_card_step.
 * @return CardaSequenceResult phase code.
 * @note g_card_step walks one of the step byte tables at g_carda_steps_initial_scan; each
 *       CardaCardStep opcode issues or polls a card command, writes or reads a
 *       save file, or scans the card directory, and updates g_card_entry_state /
 *       g_carda_selection_status. New saves are written under a dummy filename first and then
 *       renamed. Opcodes with no case are no-ops.
 */
s32 carda_advance_card_sequence(void)
{
    CardaLoadScratch card_path;
    CardaLoadScratch file_name;
    CardaLoadScratch device_path;
    u8 title_frame[0x80];
    struct DIRENTRY dir_entry;
    s32 attempts;
    s32 check_attempts;
    s32 rank_index;
    s32 rank_fill;
    s32 poll_result;
    s32 phase_result;
    long status0;
    long status1;
    s32 nibble;
    s32 started;
    s32 serial;
    s32 digits_left;
    s32 digit_index;
    s8* digit_out;
    u8* title_src;
    u8* title_dst;

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_card_slot;
    strcpy(device_path.bytes, card_path.bytes);
    phase_result = CARDA_SEQUENCE_WAIT;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        if (g_card_step == g_carda_steps_initial_scan)
        {
            g_card_step = g_carda_steps_initial_scan_check_type;
        }
    }
    if (g_card_step != NULL)
    {
        switch (*g_card_step)
        {
        case CARDA_STEP_CARD_INFO:
            phase_result = CARDA_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_info(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case CARDA_STEP_POLL_CARD_INFO:
            poll_result = poll_software_card_events();
            switch (poll_result)
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                phase_result = CARDA_SEQUENCE_NO_CARD;
                g_carda_selection_status = CARDA_SELECTION_NONE;
                g_card_entry_state = CARDA_ENTRY_STATE_NO_CARD;
                g_card_step++;
                carda_deactivate_primary_element();
                break;
            case CARD_EVENT_NEW_CARD:
                g_carda_rank_count = 0x28;
                rank_fill = -1;
                for (rank_index = 16; rank_index >= 0; rank_index--)
                {
                    g_carda_entry_ranks[rank_index] = rank_fill;
                }
                g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_carda_steps_initial_scan;
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

        case CARDA_STEP_CLEAR_SOFTWARE_EVENTS:
            clear_software_card_events();
            g_card_step++;
            break;

        case CARDA_STEP_POLL_HARDWARE_EVENTS:
            do
            {
                poll_result = poll_hardware_card_events();
            } while (poll_result == CARD_EVENT_NONE);
            switch (poll_result)
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                phase_result = CARDA_SEQUENCE_NO_CARD;
                g_carda_selection_status = CARDA_SELECTION_NONE;
                g_card_entry_state = CARDA_ENTRY_STATE_NO_CARD;
                break;
            }
            break;

        case CARDA_STEP_CLEAR_HARDWARE_EVENTS:
            clear_hardware_card_events();
            g_card_step++;
            break;

        case CARDA_STEP_SCAN_ENTRIES:
            carda_erase_placeholder_files();
            g_carda_entry_scan_active = 1;
            if (carda_begin_entry_scan(g_card_slot) == 0)
            {
                phase_result = CARDA_SEQUENCE_FINISHED;
                g_card_entry_state = CARDA_ENTRY_STATE_NO_GAME_DATA;
                g_card_step = NULL;
                g_carda_entry_scan_active = 0;
                break;
            }
            g_card_step++;
            for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
            {
                if (carda_scan_next_entry(g_card_slot) == 0)
                {
                    g_carda_entry_scan_active = 0;
                    if (g_card_entry_state != CARDA_ENTRY_STATE_NO_GAME_DATA && g_card_entry_state != CARDA_ENTRY_STATE_CARD_FULL)
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

        case CARDA_STEP_CARD_CLEAR:
            phase_result = CARDA_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_clear(CARD_CHANNEL(g_card_slot));
            g_card_step++;
            break;

        case CARDA_STEP_CARD_LOAD:
            phase_result = CARDA_SEQUENCE_RUN_AGAIN;
            _card_wait(g_card_slot);
            _card_load(CARD_CHANNEL(g_card_slot));
            g_carda_primary_poll_countdown = 0x10;
            g_carda_secondary_poll_countdown = 0x10;
            g_card_step++;
            break;

        case CARDA_STEP_DONE:
            phase_result = CARDA_SEQUENCE_FINISHED;
            g_carda_save_in_progress = 0;
            break;

        case CARDA_STEP_SKIP:
            g_card_step++;
            break;

        case CARDA_STEP_CREATE_TEMP_SAVE:
            if (g_carda_preserve_old_save == 0)
            {
                strcat(card_path.bytes, g_card_entries[g_card_slot][g_carda_selected_row].name);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&card_path) != 0)
                    {
                        break;
                    }
                }
                clear_software_card_events();
                clear_hardware_card_events();
                strcpy(card_path.bytes, device_path.bytes);
            }
            strcpy(file_name.bytes, g_lom_save_dummy_filename);
            strcat(card_path.bytes, file_name.bytes);
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&card_path, CARDA_FILE_CREATE | CARDA_FILE_BLOCKS(2));
            if (g_carda_file_handle == -1)
            {
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, card_path.bytes);
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_SAVE:
            _card_wait(g_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            clear_software_card_events();
            g_carda_progress_bar_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            _card_wait(g_card_slot);
            if (write(g_carda_file_handle, g_carda_save_blob, CARDA_SAVE_BYTES) == -1)
            {
                close(g_carda_file_handle);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_carda_temp_card_path) != 0)
                    {
                        break;
                    }
                }
                g_carda_retry_count--;
                if (g_carda_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_TEMP_SAVE and try again. */
                    g_card_step--;
                    break;
                }
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_TEMP_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                if (g_carda_preserve_old_save != 0)
                {
                    strcpy(card_path.bytes, device_path.bytes);
                    strcat(card_path.bytes, g_card_entries[g_card_slot][g_carda_selected_row].name);
                    _card_wait(g_card_slot);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(&card_path) != 0)
                        {
                            break;
                        }
                    }
                    clear_software_card_events();
                    clear_hardware_card_events();
                }

                /* Final name: prefix, serial in hex without leading zeros, then +/- and the suffix digit. */
                strcpy(card_path.bytes, device_path.bytes);
                strcpy(file_name.bytes, g_lom_save_filename_prefix);
                digit_out = (s8*)&file_name.bytes[CARD_SAVE_FILENAME_PREFIX_LENGTH];
                serial = g_carda_next_save_serial;
                digits_left = CARD_SERIAL_DIGITS;
                digit_index = 7;
                started = 0;
                while (digits_left != 0)
                {
                    nibble = (serial >> (digit_index * 4)) & 0xF;
                    if (nibble != 0 || started != 0)
                    {
                        hex_nibble_to_ascii(digit_out, nibble);
                        digit_out++;
                        digits_left--;
                        started = 1;
                        serial -= nibble << (digit_index * 4);
                    }
                    digit_index--;
                    if (digit_index == -1)
                    {
                        break;
                    }
                    if (digit_index == 0)
                    {
                        started = 1;
                    }
                }
                *digit_out = 0;
                strcat(card_path.bytes, file_name.bytes);
                if (carda_test_option_flag_2() != 0)
                {
                    file_name.bytes[0] = '+';
                }
                else
                {
                    file_name.bytes[0] = '-';
                }
                file_name.bytes[1] = g_card_entry_suffix_values[g_carda_selected_row] + '0';
                file_name.bytes[2] = 0;
                strcat(card_path.bytes, file_name.bytes);

                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_carda_temp_card_path, &card_path) != 0)
                    {
                        break;
                    }
                }

                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (firstfile(&card_path, &dir_entry) != 0)
                    {
                        break;
                    }
                }
                if (attempts != CARDA_FILE_OP_ATTEMPTS)
                {
                    /* Patch the title in the file's first sector. */
                    _card_wait(g_card_slot);
                    _card_read(CARD_CHANNEL(g_card_slot), dir_entry.head, title_frame);
                    _card_wait(g_card_slot);
                    title_dst = &title_frame[CARDA_TITLE_OFFSET];
                    title_src = &g_carda_save_title_template[0];
                    attempts = 0;
                    while (attempts < CARDA_TITLE_TEMPLATE_BYTES)
                    {
                        attempts++;
                        *title_dst++ = *title_src++;
                    }
                    /* Put a Shift-JIS note sign (0x81F4) into the fifth title character. */
                    if (carda_test_option_flag_2() != 0)
                    {
                        title_frame[0xC] = 0x81;
                        title_frame[0xD] = 0xF4;
                    }
                    _card_wait(g_card_slot);
                    _card_write(CARD_CHANNEL(g_card_slot), dir_entry.head, title_frame);
                    _card_wait(g_card_slot);
                }
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_card_step++;
                close(g_carda_file_handle);
                g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
                g_card_step = g_carda_steps_initial_scan;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_carda_retry_count--;
                if (g_carda_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_TEMP_SAVE and try again. */
                    close(g_carda_file_handle);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    g_card_step -= 2;
                }
                else
                {
                    close(g_carda_file_handle);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    g_carda_progress_bar_active = 0;
                    g_carda_progress_start_tick = VSync(-1);
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            break;

        case CARDA_STEP_CREATE_POCKETSTATION_SAVE:
            g_carda_progress_bar_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) != 0)
            {
                strcat(card_path.bytes, g_card_entries[g_card_slot][g_carda_selected_row].name);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&card_path) != 0)
                    {
                        break;
                    }
                }
                clear_software_card_events();
                clear_hardware_card_events();
            }
            strcpy(card_path.bytes, device_path.bytes);
            strcat(card_path.bytes, g_lom_pocketstation_filename_prefix);
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&card_path, CARDA_FILE_CREATE | CARDA_FILE_BLOCKS(6));
            if (g_carda_file_handle == -1)
            {
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    close(-1);
                    carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, card_path.bytes);
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_POCKETSTATION_SAVE:
            _card_wait(g_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            _card_wait(g_card_slot);
            if (write(g_carda_file_handle, g_carda_save_blob, CARDA_POCKETSTATION_SAVE_BYTES) == -1)
            {
                func_80033E7C(CARD_CHANNEL(g_card_slot));
                close(g_carda_file_handle);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_carda_temp_card_path) != 0)
                    {
                        break;
                    }
                }
                g_carda_retry_count--;
                if (g_carda_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_POCKETSTATION_SAVE and try again. */
                    g_card_step--;
                    break;
                }
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_POCKETSTATION_SAVE_WRITE:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_card_step++;
                close(g_carda_file_handle);
                g_card_step = g_carda_steps_initial_scan;
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (firstfile(g_carda_temp_card_path, &dir_entry) != 0)
                    {
                        break;
                    }
                }
                if (attempts == CARDA_FILE_OP_ATTEMPTS)
                {
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (func_80034648(CARD_CHANNEL(g_card_slot), dir_entry.head / CARDA_SECTORS_PER_BLOCK, 1) != 0)
                    {
                        break;
                    }
                }
                if (attempts != CARDA_FILE_OP_ATTEMPTS)
                {
                    return phase_result;
                }
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_carda_temp_card_path) != 0)
                    {
                        break;
                    }
                }
                clear_software_card_events();
                clear_hardware_card_events();
                carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                return phase_result;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_carda_retry_count--;
                if (g_carda_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_POCKETSTATION_SAVE and try again. */
                    close(g_carda_file_handle);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    g_card_step -= 2;
                }
                else
                {
                    close(g_carda_file_handle);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    clear_software_card_events();
                    clear_hardware_card_events();
                    carda_open_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    g_carda_progress_bar_active = 0;
                    g_carda_progress_start_tick = VSync(-1);
                }
                break;
            }
            break;

        case CARDA_STEP_POLL_CARD_LOAD:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_card_step++;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
                g_carda_secondary_poll_countdown--;
                if (g_carda_secondary_poll_countdown != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    clear_software_card_events();
                    _card_load(CARD_CHANNEL(g_card_slot));
                    break;
                }
                phase_result = CARDA_SEQUENCE_NO_CARD;
                g_carda_selection_status = CARDA_SELECTION_NONE;
                g_card_entry_state = CARDA_ENTRY_STATE_NO_CARD;
                break;
            case CARD_EVENT_NEW_CARD:
                g_carda_primary_poll_countdown--;
                if (g_carda_primary_poll_countdown != 0)
                {
                    _card_wait(g_card_slot);
                    _card_clear(CARD_CHANNEL(g_card_slot));
                    _card_wait(g_card_slot);
                    clear_software_card_events();
                    _card_load(CARD_CHANNEL(g_card_slot));
                    break;
                }
                phase_result = CARDA_SEQUENCE_UNFORMATTED;
                g_card_entry_state = CARDA_ENTRY_STATE_NO_SAVE_DATA;
                g_card_step = g_card_steps_idle;
                break;
            }
            break;

        case CARDA_STEP_CARD_WAIT:
            _card_wait(g_card_slot);
            g_card_step++;
            break;

        case CARDA_STEP_READ_HEADER:
            g_carda_io_busy = 1;
            g_carda_selection_status = CARDA_SELECTION_NONE;
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            if (g_carda_file_handle != -1)
            {
                clear_software_card_events();
                _card_wait(g_card_slot);
                if (read(g_carda_file_handle, &g_carda_selected_file,
                         g_carda_selected_entry_extended != 0 ? CARDA_ENTRY_READ_BYTES : CARDA_ENTRY_TITLE_READ_BYTES) == -1)
                {
                    close(g_carda_file_handle);
                    return phase_result;
                }
                g_card_step++;
            }
            break;

        case CARDA_STEP_POLL_HEADER_READ:
            poll_result = poll_software_card_events();
            if (poll_result == CARD_EVENT_COMPLETE)
            {
                g_carda_io_busy = 0;
                g_carda_selection_status = CARDA_SELECTION_ENTRY_READ;
                g_card_step++;
                close(g_carda_file_handle);
            }
            else if (poll_result != CARD_EVENT_NONE)
            {
                g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
                g_carda_io_busy = 0;
                close(g_carda_file_handle);
                g_card_step = g_carda_steps_initial_scan;
            }
            break;

        case CARDA_STEP_READ_SAVE:
            g_carda_progress_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_carda_file_handle, g_carda_save_blob, CARDA_SAVE_BYTES) == -1)
            {
                carda_open_status_dialog(CARDA_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_SAVE_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_carda_progress_active = 0;
                g_card_step++;
                close(g_carda_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_bar_active = 0;
                carda_open_status_dialog(CARDA_DIALOG_LOAD_FAILED);
                break;
            }
            break;

        case CARDA_STEP_CHECK_POCKETSTATION:
            for (check_attempts = 0; check_attempts < CARDA_FILE_OP_ATTEMPTS; check_attempts++)
            {
                for (attempts = 0; attempts < 120; attempts++)
                {
                    if (McxCardType(CARD_CHANNEL(g_card_slot)) == MCX_COMMAND_ISSUED)
                    {
                        break;
                    }
                    VSync(0);
                }
                VSync(0);
                /* The wait above allows 120 frames, but only 20 counts as a timeout. */
                if (attempts != 20)
                {
                    McxSync(MCX_SYNC_WAIT, &status0, &status1);
                    switch (status1)
                    {
                    case McxErrSuccess:
                        g_card_step++;
                        return phase_result;
                    case McxErrNoCard:
                        g_card_entry_state = CARDA_ENTRY_STATE_NO_CARD;
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
            g_carda_progress_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            clear_software_card_events();
            _card_wait(g_card_slot);
            if (read(g_carda_file_handle, g_carda_save_blob, CARDA_SAVE_PREFIX_BYTES) == -1)
            {
                carda_open_save_status_dialog(CARDA_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            g_card_step++;
            break;

        case CARDA_STEP_POLL_SAVE_PREFIX_READ:
            switch (poll_software_card_events())
            {
            case CARD_EVENT_COMPLETE:
                g_carda_progress_active = 0;
                g_card_step++;
                close(g_carda_file_handle);
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_carda_progress_bar_active = 0;
                carda_open_save_status_dialog(CARDA_DIALOG_LOAD_FAILED);
                return phase_result;
            }
            break;

        case CARDA_STEP_ARM_RETRIES:
            g_carda_retry_count = CARDA_SAVE_RETRIES;
            g_card_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_POCKETSTATION_SAVE:
            if (g_carda_preserve_old_save == 0)
            {
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&g_carda_selected_card_path) != 0)
                    {
                        break;
                    }
                }
            }
            strcat(card_path.bytes, g_lom_pocketstation_dummy_filename);
            _card_wait(g_card_slot);
            g_carda_file_handle = open(&card_path, CARDA_FILE_CREATE | CARDA_FILE_BLOCKS(6));
            if (g_carda_file_handle == -1)
            {
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    close(-1);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(&card_path) != 0)
                        {
                            break;
                        }
                    }
                    carda_open_save_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, card_path.bytes);
            _card_wait(g_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            clear_software_card_events();
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_card_slot);
            if (write(g_carda_file_handle, g_carda_save_blob, CARDA_POCKETSTATION_SAVE_BYTES) == -1)
            {
                close(g_carda_file_handle);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_carda_temp_card_path) != 0)
                    {
                        break;
                    }
                }
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    carda_open_save_status_dialog(CARDA_DIALOG_SAVE_FAILED);
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
                if (g_carda_preserve_old_save != 0)
                {
                    _card_wait(g_card_slot);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(&g_carda_selected_card_path) != 0)
                        {
                            break;
                        }
                    }
                }
                _card_wait(g_card_slot);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_carda_temp_card_path, &g_carda_selected_card_path) != 0)
                    {
                        break;
                    }
                }
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_card_step++;
                close(g_carda_file_handle);
                g_card_step = g_carda_steps_initial_scan;
                break;
            case CARD_EVENT_ERROR:
            case CARD_EVENT_TIMEOUT:
            case CARD_EVENT_NEW_CARD:
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    close(g_carda_file_handle);
                    g_carda_progress_bar_active = 0;
                    carda_open_save_status_dialog(CARDA_DIALOG_SAVE_FAILED);
                    return phase_result;
                }
                /* Rewind to CARDA_STEP_WRITE_TEMP_POCKETSTATION_SAVE and try again. */
                close(g_carda_file_handle);
                g_card_step--;
                break;
            }
            break;
        }
    }
    return phase_result;
}

/**
 * @brief Wait for the current card, then reset the list to a single new-save entry.
 */
void carda_reset_to_new_save_entry(void)
{
    s32 unused[4];
    s32 attempt;

    for (attempt = 0; attempt < CARDA_FILE_OP_ATTEMPTS; attempt++)
    {
        if (_card_format(CARD_CHANNEL(g_card_slot)) != 0)
        {
            break;
        }
    }

    g_carda_preserve_old_save = 0;
    g_carda_selected_row = 0;
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
    g_carda_progress_start_tick = VSync(-1);
    g_carda_progress_bar_active = 0;
    g_carda_entry_scan_active = 0;
}

#include "../../common/card_events/shutdown_card_events.inc.c"

/**
 * @brief Reset the list view and read the first directory entry of a card.
 * @param page Memory-card slot to scan.
 * @return 1 if the first entry was read or the mode allows an empty card, 0 otherwise.
 */
static s32 carda_begin_entry_scan(s32 page)
{
    CardaCardSearchPath search_path;
    s32 attempt;

    memcpy(&search_path, &g_carda_card_search_path, 7);
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    search_path.bytes[2] += page;
    _card_wait(page);
    g_card_entry_state = 0;
    for (attempt = 0; attempt < CARDA_FILE_OP_ATTEMPTS; attempt++)
    {
        if (firstfile(&search_path, g_card_entries[page]) != 0)
        {
            field_flag_known_save(&g_card_entries[page][g_card_entry_state]);
            g_card_entry_state += 1;
            return 1;
        }
    }
    if (g_carda_mode == CARDA_MODE_LOAD || g_carda_mode == CARDA_MODE_RETURN_PET)
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
    for (entry_attempt = 0; entry_attempt < CARDA_FILE_OP_ATTEMPTS; entry_attempt++)
    {
        if (nextfile(&g_card_entries[page][g_card_entry_state]) != 0)
        {
            field_flag_known_save(&g_card_entries[page][g_card_entry_state]);
            g_card_entry_state += 1;
            return 1;
        }
    }

    field_reset_input_repeat();
    if (g_carda_mode == CARDA_MODE_LOAD && carda_has_known_entry_type() == 0)
    {
        g_card_entry_state = CARDA_ENTRY_STATE_NO_GAME_DATA;
    }
    else
    {
        g_carda_preserve_old_save = 0;
        if (carda_card_lacks_free_blocks())
        {
            if (g_carda_mode == CARDA_MODE_SAVE || g_carda_mode == CARDA_MODE_SEND_PET)
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_card_full_entry_name);
                g_card_entries[page][g_card_entry_state].size = 0;
                g_card_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
            {
                if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
                {
                    g_card_entry_state = CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD;
                }
                else
                {
                    g_card_entry_state = CARDA_ENTRY_STATE_CARD_FULL;
                }
                g_carda_next_save_serial = 0;
            }
            else
            {
                g_carda_selected_row = selected;
                carda_scroll_to_selection();
            }
        }
        else
        {
            g_carda_preserve_old_save = 1;
            if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_new_save_entry_prefix);
                g_card_entries[page][g_card_entry_state].size = 0xC000;
                g_card_entry_state += 1;
            }
            else if (g_carda_mode == CARDA_MODE_SAVE)
            {
                strcpy(g_card_entries[page][g_card_entry_state].name, g_new_save_entry_prefix);
                g_card_entries[page][g_card_entry_state].size = 0x4000;
                g_card_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
            {
                g_carda_selected_row = 0;
                carda_scroll_to_selection();
                g_carda_next_save_serial = 0;
            }
            else
            {
                g_carda_selected_row = selected;
                carda_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Validate the selected entry and start reading its file header.
 * @note Sets g_carda_selection_status to 3 for an empty card, 2 for the new-save entry, 4 for the
 *       full-card entry, otherwise builds "buX0:<name>" into g_carda_selected_card_path and arms g_carda_steps_read_selected_header.
 */
void carda_commit_selected_entry(void)
{
    CardaCardPathBuffer card_path;

    if (g_card_entry_state == 0)
    {
        g_carda_selection_status = CARDA_SELECTION_EMPTY_CARD;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) == 0)
    {
        g_carda_selection_status = CARDA_SELECTION_NEW_SAVE;
        return;
    }
    if (strncmp(g_card_full_entry_name, g_card_entries[g_card_slot][g_carda_selected_row].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) == 0)
    {
        g_carda_selection_status = CARDA_SELECTION_CARD_FULL;
        return;
    }
    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    strcat(card_path.bytes, g_card_entries[g_card_slot][g_carda_selected_row].name);
    card_path.bytes[2] += (u8)g_card_slot;
    g_carda_selection_status = CARDA_SELECTION_NONE;
    strcpy(g_carda_selected_card_path.raw, card_path.bytes);
    g_card_step = &g_carda_steps_read_selected_header[0];
    g_carda_io_busy = 1;
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_carda_selected_entry_extended = 1;
    }
    else
    {
        g_carda_selected_entry_extended = 0;
    }
}

#include "../../common/card_events/clear_software_card_events.inc.c"
#include "../../common/card_events/clear_hardware_card_events.inc.c"
#include "../../common/card_events/poll_software_card_events.inc.c"
#include "../../common/card_events/poll_hardware_card_events.inc.c"

/**
 * @brief Reorder the current card's directory by save type and suffix byte.
 * @note g_lom_save_filename_prefix saves come first, then g_lom_pocketstation_filename_prefix saves (each by suffix group),
 *       then the placeholder entries, then everything else.
 */
static void carda_sort_entries_by_type(void)
{
    struct DIRENTRY sorted_entries[CARD_DIRECTORY_ENTRY_COUNT];
    s32 output_count = 0;
    s32 group;
    s32 entry_index;

    for (group = 0; group < CARDA_ENTRY_GROUP_COUNT; group++)
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

    for (group = 0; group < CARDA_ENTRY_GROUP_COUNT; group++)
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
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) == 0 ||
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
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) != 0 &&
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
