#include "carda_internal.h"
#include "sdk/kernel.h"

/**
 * @brief Address of the hex field at name offset 0xC of directory entry @p index on card @p card.
 * @note Summed as integers, offsets first, like the original code.
 */
#define CARDA_ENTRY_FIELD_TEXT(card, index) \
    ((u8 *)((card) * CARDA_CARD_DIRECTORY_BYTES + (index) * CARDA_DIRECTORY_ENTRY_BYTES + (s32)g_carda_entries + 0xC))

/**
 * @brief Format up to six decimal digits as full-width Shift-JIS characters.
 * @param out Destination byte buffer.
 * @param value Number to format.
 * @return Pointer to the terminator, or six bytes past the start for the overflow text.
 * @note Values of 1000000 or more use the fixed overflow text g_carda_decimal_overflow_text; leading zeroes are suppressed.
 */
s8 *carda_format_decimal(s8 *out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8 *cursor;

    cursor = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(CardaDecimalOverflow *)cursor = g_carda_decimal_overflow_text;
        return cursor + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *cursor++ = (digit + 0x824F) >> 8;
            *cursor++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *cursor = 0;
    return cursor;
}

/**
 * @brief Format a hexadecimal string with leading zeroes suppressed.
 * @param out Destination character buffer; receives the digits and a terminator.
 * @param value Number to format.
 * @param max_chars Maximum number of digits to emit, excluding the terminator.
 */
void carda_format_hex(s8 *out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 started;

    shift_index = 7;
    started = 0;
    if (max_chars != 0)
    {
        do
        {
            nibble = (value >> (shift_index * 4)) & 0xF;
            if (nibble != 0 || started != 0)
            {
                carda_hex_nibble_to_ascii(out, nibble);
                out++;
                max_chars--;
                started = 1;
                value -= nibble << (shift_index * 4);
            }
            shift_index--;
            if (shift_index == -1)
            {
                break;
            }
            if (shift_index == 0)
            {
                started = 1;
            }
        } while (max_chars);
    }
    *out = 0;
}

/**
 * @brief Convert a 0-15 value to its ASCII hexadecimal digit.
 * @param out Destination byte.
 * @param value Nibble value; 0-9 give '0'-'9', 10-15 give 'A'-'F', anything else '_'.
 */
void carda_hex_nibble_to_ascii(s8 *out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

/**
 * @brief Parse a bounded run of ASCII hexadecimal digits.
 * @param text First digit to parse.
 * @param digits_left Maximum number of digits to consume.
 * @return Accumulated value; parsing stops at the digit limit or the first non-hex byte.
 */
u32 carda_parse_hex(u8 *text, s32 digits_left)
{
    u32 result;

    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *text;
        }
        text++;
        digits_left--;
    }
    return result;
}

/**
 * @brief Skip a hexadecimal run and its separator, then parse up to two hex digits.
 * @param text Start of the leading hexadecimal run.
 * @return Parsed suffix byte.
 */
s32 carda_parse_hex_suffix_byte(u8 *text)
{
    s32 digits_left;
    u32 result;

    while ((u32)(*text - '0') < 10 || (u32)(*text - 'a') < 6 || (u32)(*text - 'A') < 6)
    {
        text++;
    }

    text++;
    digits_left = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *text;
        }
        text++;
        digits_left--;
    }
    return result;
}

/**
 * @brief Parse the field value and suffix byte of every recognized save-file name.
 *
 * Entries whose name starts with g_lom_save_filename_prefix have up to five hex digits at name
 * offset 0xC parsed into g_carda_entry_fields; the suffix byte after that run is parsed
 * by carda_parse_hex_suffix_byte into g_carda_entry_suffix_values. Other entries store -1 / 0 instead.
 *
 * @return Largest suffix byte among the recognized entries (0 if none).
 */
s32 carda_parse_entry_fields(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8 *cursor;
    u8 *suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;
    s32 suffix_value;

    entry_index = 0;
    max_suffix = entry_index;
    while (entry_index < g_carda_entry_state)
    {
        if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
        {
            digits_left = 5;
            cursor = CARDA_ENTRY_FIELD_TEXT(g_carda_card_slot, entry_index);
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
            suffix = (u8 *)&g_carda_entries[g_carda_card_slot][entry_index].name[0xC];
            g_carda_entry_fields[g_carda_card_slot][entry_index] = value;
            suffix_value = carda_parse_hex_suffix_byte(suffix);
            g_carda_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            g_carda_entry_fields[g_carda_card_slot][entry_index] = -1;
            g_carda_entry_suffix_values[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}

/**
 * @brief Sort and rank the current card's entries and number the new-save entry.
 *
 * Recognized saves are ranked by their field value into g_carda_entry_ranks, the next
 * field value is stored in D_80166AD8, and the new-save placeholder entry gets
 * the lowest suffix byte (1-8) that no recognized save uses.
 *
 * @return Index of the entry holding the greatest field value.
 */
s32 carda_rank_entries(void)
{
    s32 suffix_used[10];
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;
    u8 *cursor;
    u8 *suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;

    carda_parse_entry_fields();
    carda_sort_entries_by_type();
    for (entry_index = 9; entry_index >= 0; entry_index--)
    {
        suffix_used[entry_index] = 0;
    }

    max_suffix = 0;
    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
        {
            digits_left = 5;
            cursor = CARDA_ENTRY_FIELD_TEXT(g_carda_card_slot, entry_index);
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
            suffix = (u8 *)&g_carda_entries[g_carda_card_slot][entry_index].name[0xC];
            g_carda_entry_fields[g_carda_card_slot][entry_index] = value;
            g_carda_entry_suffix_values[entry_index] = carda_parse_hex_suffix_byte(suffix);
            suffix_used[g_carda_entry_suffix_values[entry_index]] = 1;
            if (max_suffix < g_carda_entry_suffix_values[entry_index])
            {
                max_suffix = g_carda_entry_suffix_values[entry_index];
            }
        }
        else
        {
            g_carda_entry_fields[g_carda_card_slot][entry_index] = -1;
            g_carda_entry_suffix_values[entry_index] = 0;
        }
    }

    maximum = -1;
    carda_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (g_carda_entry_fields[g_carda_card_slot][entry_index] >= 0)
        {
            if (g_carda_entry_fields[g_carda_card_slot][entry_index] >= maximum)
            {
                g_carda_entry_ranks[entry_index] = next_rank;
                maximum = g_carda_entry_fields[g_carda_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_carda_entry_fields[g_carda_card_slot][entry_index] < g_carda_entry_fields[g_carda_card_slot][previous_index])
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
    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (next_rank < g_carda_entry_fields[g_carda_card_slot][entry_index])
        {
            next_rank = g_carda_entry_fields[g_carda_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    D_80166AD8 = next_rank + 1;

    /* Reuse max_suffix as the lowest suffix no recognized entry uses. */
    for (max_suffix = 1; max_suffix < 9; max_suffix++)
    {
        if (suffix_used[max_suffix] == 0)
        {
            break;
        }
    }
    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 8) == 0)
        {
            g_carda_entry_suffix_values[entry_index] = max_suffix;
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
 * @return 1 if an entry matches g_lom_save_filename_prefix or g_lom_alt_save_filename_prefix (only g_lom_alt_save_filename_prefix in mode 2), otherwise 0.
 */
s32 carda_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (g_carda_mode != 2)
        {
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0 ||
                strncmp(g_lom_alt_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
            {
                return 1;
            }
        }
        if (g_carda_mode == 2 && strncmp(g_lom_alt_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
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
    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        used_blocks += g_carda_entries[g_carda_card_slot][entry_index].size / CARDA_MEMORY_CARD_BLOCK_BYTES;
    }
    if ((used_blocks < 14 && g_carda_mode != 2) || (used_blocks < 10 && g_carda_mode == 2))
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
inline void carda_erase_fixed_card_files(void)
{
    CardaLoadScratch card_path;

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_carda_card_slot;
    strcat(&card_path, g_lom_save_dummy_filename);
    erase(&card_path);

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_carda_card_slot;
    strcat(&card_path, g_lom_alt_save_dummy_filename);
    erase(&card_path);
    carda_release_primary_handles();
    carda_release_secondary_handles();
}

/**
 * @brief Save-sequence step opcodes stored in the g_carda_steps_initial_scan..g_carda_steps_read_save_prefix byte tables.
 * @note Opcodes without a case (7, 14) are no-ops; 14 is the idle opcode that
 *       the g_carda_steps_idle table parks on.
 */
typedef enum CardaSaveStep
{
    CARDA_STEP_DONE = 0,                    /**< End of a step table; report phase 2. */
    CARDA_STEP_CARD_INFO = 1,               /**< Issue _card_info on the current slot. */
    CARDA_STEP_POLL_CARD_INFO = 2,          /**< Wait for the _card_info result. */
    CARDA_STEP_RELEASE_PRIMARY = 3,         /**< Clear the primary card event group. */
    CARDA_STEP_WAIT_SECONDARY = 4,          /**< Wait for and check the secondary card events. */
    CARDA_STEP_RELEASE_SECONDARY = 5,       /**< Clear the secondary card event group. */
    CARDA_STEP_SCAN_ENTRIES = 6,            /**< Erase the dummy files and scan the card directory. */
    CARDA_STEP_CARD_CLEAR = 8,              /**< Issue _card_clear on the current slot. */
    CARDA_STEP_CARD_LOAD = 9,               /**< Issue _card_load and arm the poll countdowns. */
    CARDA_STEP_SKIP = 10,                   /**< Advance without doing anything; entry point of the write tables. */
    CARDA_STEP_CREATE_TEMP_SAVE = 11,       /**< Create the two-block save under the dummy filename. */
    CARDA_STEP_WRITE_TEMP_SAVE = 12,        /**< Start writing the save blob into the dummy file. */
    CARDA_STEP_POLL_TEMP_SAVE_WRITE = 13,   /**< Wait for the write, rename the file and patch its title. */
    CARDA_STEP_IDLE = 14,                   /**< No-op step that never advances. */
    CARDA_STEP_POLL_CARD_LOAD = 15,         /**< Wait for the _card_clear/_card_load result, retrying. */
    CARDA_STEP_CARD_WAIT = 16,              /**< Wait for the pending card command to finish. */
    CARDA_STEP_READ_HEADER = 17,            /**< Open the selected save and start reading its header. */
    CARDA_STEP_POLL_HEADER_READ = 18,       /**< Wait for the header read to finish. */
    CARDA_STEP_READ_SAVE = 19,              /**< Open the selected save and start reading the blob. */
    CARDA_STEP_POLL_SAVE_READ = 20,         /**< Wait for the blob read to finish. */
    CARDA_STEP_CREATE_ALT_SAVE = 21,        /**< Create the six-block alternate save file. */
    CARDA_STEP_WRITE_ALT_SAVE = 22,         /**< Start writing the alternate save file. */
    CARDA_STEP_POLL_ALT_SAVE_WRITE = 23,    /**< Wait for the alternate save write and finalize the file. */
    CARDA_STEP_CHECK_CARD_TYPE = 24,        /**< Wait for a card and check its status. */
    CARDA_STEP_POLL_CARD_PRESENT = 25,      /**< Wait for the _card_info result; report a failure. */
    CARDA_STEP_WRITE_TEMP_ALT_SAVE = 26,    /**< Create the alternate dummy file and start writing it. */
    CARDA_STEP_POLL_TEMP_ALT_SAVE_WRITE = 27, /**< Wait for the write and rename it over the selected save. */
    CARDA_STEP_READ_SAVE_PREFIX = 28,       /**< Open the selected save and start reading its first 1 KiB. */
    CARDA_STEP_POLL_SAVE_PREFIX_READ = 29,  /**< Wait for the 1 KiB read to finish. */
    CARDA_STEP_ARM_RETRIES = 30             /**< Arm the file operation retry counter. */
} CardaSaveStep;

/**
 * @brief Result index returned by the card event group pollers.
 * @note Matches the order of the SwCARD IOE/ERROR/TIMEOUT/NEWCARD events.
 */
typedef enum CardaCardEvent
{
    CARDA_CARD_EVENT_READY = 0,     /**< Operation completed. */
    CARDA_CARD_EVENT_ERROR = 1,     /**< Card reported an error. */
    CARDA_CARD_EVENT_TIMEOUT = 2,   /**< No card or no response. */
    CARDA_CARD_EVENT_NEW_CARD = 3   /**< A different card was inserted. */
} CardaCardEvent;

/**
 * @brief Phase codes returned by carda_advance_save_sequence.
 */
typedef enum CardaSaveResult
{
    CARDA_SAVE_CONTINUE = 1,      /**< Step handled; poll again next frame. */
    CARDA_SAVE_FINISHED = 2,      /**< Step table ended. */
    CARDA_SAVE_REPEAT = 3,        /**< Card command issued; run the next step now. */
    CARDA_SAVE_REFRESH = 4,       /**< Card missing or failed; refresh the entry list. */
    CARDA_SAVE_CARD_CHANGED = 5   /**< Card replaced during the sequence. */
} CardaSaveResult;

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

/** @brief Size of the alternate save file (six memory-card blocks). */
#define CARDA_ALT_SAVE_BYTES 0xC000

/** @brief Leading part of a save read back by CARDA_STEP_READ_SAVE_PREFIX. */
#define CARDA_SAVE_PREFIX_BYTES 0x400

/** @brief Number of 128-byte sectors in one memory-card block. */
#define CARDA_SECTORS_PER_BLOCK 64

/** @brief Attempts made at a synchronous card file operation before giving up. */
#define CARDA_FILE_OP_ATTEMPTS 20

/** @brief Retries of a failed asynchronous save write (CARDA_STEP_ARM_RETRIES). */
#define CARDA_SAVE_RETRIES 5

/** @brief Length of the save filename prefix (g_lom_save_filename_prefix) before the hex serial. */
#define CARDA_SAVE_PREFIX_LENGTH 12

/** @brief Number of hex digits in a save filename serial. */
#define CARDA_SERIAL_DIGITS 5

/** @brief Length of the title template (g_carda_save_title_template) copied into a save's title frame. */
#define CARDA_TITLE_TEMPLATE_BYTES 18

/** @brief Offset of the title in a memory-card title frame. */
#define CARDA_TITLE_OFFSET 4

/**
 * @brief Run the current memory-card save step and advance g_carda_save_step.
 * @return CardaSaveResult phase code.
 * @note g_carda_save_step walks one of the step byte tables at g_carda_steps_initial_scan; each
 *       CardaSaveStep opcode issues or polls a card command, writes or reads a
 *       save file, or scans the card directory, and updates g_carda_entry_state /
 *       g_carda_selection_status. New saves are written under a dummy filename first and then
 *       renamed. Opcodes with no case are no-ops.
 */
s32 carda_advance_save_sequence(void)
{
    CardaLoadScratch card_path;
    CardaLoadScratch file_name;
    CardaLoadScratch device_path;
    u8 title_frame[0x80];
    CardaDirEntry dir_entry;
    s32 attempts;
    s32 check_attempts;
    s32 rank_index;
    s32 rank_fill;
    s32 poll_result;
    s32 phase_result;
    s32 status0;
    s32 status1;
    s32 nibble;
    s32 started;
    s32 serial;
    s32 digits_left;
    s32 digit_index;
    s8 *digit_out;
    u8 *title_src;
    u8 *title_dst;

    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_carda_card_slot;
    strcpy(&device_path, &card_path);
    phase_result = CARDA_SAVE_CONTINUE;
    if (g_carda_mode == 2 || g_carda_mode == 3)
    {
        if (g_carda_save_step == g_carda_steps_initial_scan)
        {
            g_carda_save_step = g_carda_steps_initial_scan_check_type;
        }
    }
    if (g_carda_save_step != NULL)
    {
        switch (*g_carda_save_step)
        {
        case CARDA_STEP_CARD_INFO:
            phase_result = CARDA_SAVE_REPEAT;
            _card_wait(g_carda_card_slot);
            _card_info(g_carda_card_slot * 0x10);
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_CARD_INFO:
            poll_result = carda_poll_primary_handle_group();
            switch (poll_result)
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_save_step++;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
                phase_result = CARDA_SAVE_REFRESH;
                g_carda_selection_status = 0;
                g_carda_entry_state = 0xFD;
                g_carda_save_step++;
                carda_deactivate_primary_element();
                break;
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_rank_count = 0x28;
                rank_fill = -1;
                for (rank_index = 16; rank_index >= 0; rank_index--)
                {
                    g_carda_entry_ranks[rank_index] = rank_fill;
                }
                g_carda_entry_state = 0xFF;
                g_carda_save_step = g_carda_steps_initial_scan;
                break;
            }
            break;

        case CARDA_STEP_POLL_CARD_PRESENT:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_save_step++;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                carda_open_status_dialog(5);
                break;
            }
            break;

        case CARDA_STEP_RELEASE_PRIMARY:
            carda_release_primary_handles();
            g_carda_save_step++;
            break;

        case CARDA_STEP_WAIT_SECONDARY:
            do
            {
                poll_result = carda_poll_secondary_handle_group();
            } while (poll_result == -1);
            switch (poll_result)
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_save_step++;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                phase_result = CARDA_SAVE_REFRESH;
                g_carda_selection_status = 0;
                g_carda_entry_state = 0xFD;
                break;
            }
            break;

        case CARDA_STEP_RELEASE_SECONDARY:
            carda_release_secondary_handles();
            g_carda_save_step++;
            break;

        case CARDA_STEP_SCAN_ENTRIES:
            carda_erase_fixed_card_files();
            g_carda_entry_scan_active = 1;
            if (carda_begin_entry_scan(g_carda_card_slot) == 0)
            {
                phase_result = CARDA_SAVE_FINISHED;
                g_carda_entry_state = 0xF8;
                g_carda_save_step = NULL;
                g_carda_entry_scan_active = 0;
                break;
            }
            g_carda_save_step++;
            for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
            {
                if (carda_scan_next_entry(g_carda_card_slot) == 0)
                {
                    g_carda_entry_scan_active = 0;
                    if (g_carda_entry_state != 0xF8 && g_carda_entry_state != 0xFA)
                    {
                        if (g_carda_entry_state != 0xF7)
                        {
                            carda_commit_selected_entry();
                        }
                    }
                    break;
                }
            }
            break;

        case CARDA_STEP_CARD_CLEAR:
            phase_result = CARDA_SAVE_REPEAT;
            _card_wait(g_carda_card_slot);
            _card_clear(g_carda_card_slot * 0x10);
            g_carda_save_step++;
            break;

        case CARDA_STEP_CARD_LOAD:
            phase_result = CARDA_SAVE_REPEAT;
            _card_wait(g_carda_card_slot);
            _card_load(g_carda_card_slot * 0x10);
            g_carda_primary_poll_countdown = 0x10;
            g_carda_secondary_poll_countdown = 0x10;
            g_carda_save_step++;
            break;

        case CARDA_STEP_DONE:
            phase_result = CARDA_SAVE_FINISHED;
            g_carda_save_in_progress = 0;
            break;

        case CARDA_STEP_SKIP:
            g_carda_save_step++;
            break;

        case CARDA_STEP_CREATE_TEMP_SAVE:
            if (g_carda_preserve_old_save == 0)
            {
                strcat(&card_path, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&card_path) != 0)
                    {
                        break;
                    }
                }
                carda_release_primary_handles();
                carda_release_secondary_handles();
                strcpy(&card_path, &device_path);
            }
            strcpy(&file_name, g_lom_save_dummy_filename);
            strcat(&card_path, &file_name);
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(&card_path, CARDA_FILE_CREATE | CARDA_FILE_BLOCKS(2));
            if (g_carda_file_handle == -1)
            {
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    carda_release_primary_handles();
                    carda_release_secondary_handles();
                    carda_open_status_dialog(0);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, &card_path);
            g_carda_save_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_SAVE:
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            carda_release_primary_handles();
            g_carda_progress_bar_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            _card_wait(g_carda_card_slot);
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
                    g_carda_save_step--;
                    break;
                }
                carda_release_primary_handles();
                carda_release_secondary_handles();
                carda_open_status_dialog(0);
                return phase_result;
            }
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_TEMP_SAVE_WRITE:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                if (g_carda_preserve_old_save != 0)
                {
                    strcpy(&card_path, &device_path);
                    strcat(&card_path, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name);
                    _card_wait(g_carda_card_slot);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(&card_path) != 0)
                        {
                            break;
                        }
                    }
                    carda_release_primary_handles();
                    carda_release_secondary_handles();
                }

                /* Final name: prefix, serial in hex without leading zeros, then +/- and the suffix digit. */
                strcpy(&card_path, &device_path);
                strcpy(&file_name, g_lom_save_filename_prefix);
                digit_out = (s8 *)&file_name.bytes[CARDA_SAVE_PREFIX_LENGTH];
                serial = D_80166AD8;
                digits_left = CARDA_SERIAL_DIGITS;
                digit_index = 7;
                started = 0;
                while (digits_left != 0)
                {
                    nibble = (serial >> (digit_index * 4)) & 0xF;
                    if (nibble != 0 || started != 0)
                    {
                        carda_hex_nibble_to_ascii(digit_out, nibble);
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
                strcat(&card_path, &file_name);
                if (carda_test_option_flag_2() != 0)
                {
                    file_name.bytes[0] = '+';
                }
                else
                {
                    file_name.bytes[0] = '-';
                }
                file_name.bytes[1] = g_carda_entry_suffix_values[g_carda_selected_row] + '0';
                file_name.bytes[2] = 0;
                strcat(&card_path, &file_name);

                _card_wait(g_carda_card_slot);
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
                    _card_wait(g_carda_card_slot);
                    _card_read(g_carda_card_slot * 0x10, dir_entry.head, title_frame);
                    _card_wait(g_carda_card_slot);
                    title_dst = &title_frame[CARDA_TITLE_OFFSET];
                    title_src = &g_carda_save_title_template[0];
                    attempts = 0;
                    do
                    {
                        attempts++;
                        *title_dst++ = *title_src++;
                    } while (attempts < CARDA_TITLE_TEMPLATE_BYTES);
                    /* Put a Shift-JIS note sign (0x81F4) into the fifth title character. */
                    if (carda_test_option_flag_2() != 0)
                    {
                        title_frame[0xC] = 0x81;
                        title_frame[0xD] = 0xF4;
                    }
                    _card_wait(g_carda_card_slot);
                    _card_write(g_carda_card_slot * 0x10, dir_entry.head, title_frame);
                    _card_wait(g_carda_card_slot);
                }
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_carda_save_step++;
                close(g_carda_file_handle);
                g_carda_entry_state = 0xFF;
                g_carda_save_step = g_carda_steps_initial_scan;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
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
                    g_carda_save_step -= 2;
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
                    carda_release_primary_handles();
                    carda_release_secondary_handles();
                    carda_open_status_dialog(0);
                    return phase_result;
                }
                break;
            }
            break;

        case CARDA_STEP_CREATE_ALT_SAVE:
            g_carda_progress_bar_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 8) != 0)
            {
                strcat(&card_path, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&card_path) != 0)
                    {
                        break;
                    }
                }
                carda_release_primary_handles();
                carda_release_secondary_handles();
            }
            strcpy(&card_path, &device_path);
            strcat(&card_path, g_lom_alt_save_filename_prefix);
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(&card_path, CARDA_FILE_CREATE | CARDA_FILE_BLOCKS(6));
            if (g_carda_file_handle == -1)
            {
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    close(-1);
                    carda_open_status_dialog(0);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, &card_path);
            g_carda_save_step++;
            break;

        case CARDA_STEP_WRITE_ALT_SAVE:
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            _card_wait(g_carda_card_slot);
            if (write(g_carda_file_handle, g_carda_save_blob, CARDA_ALT_SAVE_BYTES) == -1)
            {
                func_80033E7C(g_carda_card_slot * 0x10);
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
                    /* Rewind to CARDA_STEP_CREATE_ALT_SAVE and try again. */
                    g_carda_save_step--;
                    break;
                }
                carda_release_primary_handles();
                carda_release_secondary_handles();
                carda_open_status_dialog(0);
                return phase_result;
            }
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_ALT_SAVE_WRITE:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_carda_save_step++;
                close(g_carda_file_handle);
                g_carda_save_step = g_carda_steps_initial_scan;
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
                    carda_release_primary_handles();
                    carda_release_secondary_handles();
                    carda_open_status_dialog(0);
                    return phase_result;
                }
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (func_80034648(g_carda_card_slot * 0x10, dir_entry.head / CARDA_SECTORS_PER_BLOCK, 1) != 0)
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
                carda_release_primary_handles();
                carda_release_secondary_handles();
                carda_open_status_dialog(0);
                return phase_result;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_retry_count--;
                if (g_carda_retry_count != 0)
                {
                    /* Rewind to CARDA_STEP_CREATE_ALT_SAVE and try again. */
                    close(g_carda_file_handle);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_carda_temp_card_path) != 0)
                        {
                            break;
                        }
                    }
                    g_carda_save_step -= 2;
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
                    carda_release_primary_handles();
                    carda_release_secondary_handles();
                    carda_open_status_dialog(0);
                    g_carda_progress_bar_active = 0;
                    g_carda_progress_start_tick = VSync(-1);
                }
                break;
            }
            break;

        case CARDA_STEP_POLL_CARD_LOAD:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_save_step++;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
                g_carda_secondary_poll_countdown--;
                if (g_carda_secondary_poll_countdown != 0)
                {
                    _card_wait(g_carda_card_slot);
                    _card_clear(g_carda_card_slot * 0x10);
                    _card_wait(g_carda_card_slot);
                    carda_release_primary_handles();
                    _card_load(g_carda_card_slot * 0x10);
                    break;
                }
                phase_result = CARDA_SAVE_REFRESH;
                g_carda_selection_status = 0;
                g_carda_entry_state = 0xFD;
                break;
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_primary_poll_countdown--;
                if (g_carda_primary_poll_countdown != 0)
                {
                    _card_wait(g_carda_card_slot);
                    _card_clear(g_carda_card_slot * 0x10);
                    _card_wait(g_carda_card_slot);
                    carda_release_primary_handles();
                    _card_load(g_carda_card_slot * 0x10);
                    break;
                }
                phase_result = CARDA_SAVE_CARD_CHANGED;
                g_carda_entry_state = 0xFC;
                g_carda_save_step = g_carda_steps_idle;
                break;
            }
            break;

        case CARDA_STEP_CARD_WAIT:
            _card_wait(g_carda_card_slot);
            g_carda_save_step++;
            break;

        case CARDA_STEP_READ_HEADER:
            g_carda_io_busy = 1;
            g_carda_selection_status = 0;
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            if (g_carda_file_handle != -1)
            {
                carda_release_primary_handles();
                _card_wait(g_carda_card_slot);
                if (read(g_carda_file_handle, &g_carda_selected_file_header, g_carda_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
                {
                    close(g_carda_file_handle);
                    return phase_result;
                }
                g_carda_save_step++;
            }
            break;

        case CARDA_STEP_POLL_HEADER_READ:
            poll_result = carda_poll_primary_handle_group();
            if (poll_result == CARDA_CARD_EVENT_READY)
            {
                g_carda_io_busy = 0;
                g_carda_selection_status = 1;
                g_carda_save_step++;
                close(g_carda_file_handle);
            }
            else if (poll_result != -1)
            {
                g_carda_entry_state = 0xFF;
                g_carda_io_busy = 0;
                close(g_carda_file_handle);
                g_carda_save_step = g_carda_steps_initial_scan;
            }
            break;

        case CARDA_STEP_READ_SAVE:
            g_carda_progress_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            carda_release_primary_handles();
            _card_wait(g_carda_card_slot);
            if (read(g_carda_file_handle, g_carda_save_blob, CARDA_SAVE_BYTES) == -1)
            {
                carda_open_status_dialog(1);
                return phase_result;
            }
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_SAVE_READ:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_progress_active = 0;
                g_carda_save_step++;
                close(g_carda_file_handle);
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_bar_active = 0;
                carda_open_status_dialog(1);
                break;
            }
            break;

        case CARDA_STEP_CHECK_CARD_TYPE:
            for (check_attempts = 0; check_attempts < 20; check_attempts++)
            {
                for (attempts = 0; attempts < 120; attempts++)
                {
                    if (McxCardType(g_carda_card_slot * 0x10) == 1)
                    {
                        break;
                    }
                    VSync(0);
                }
                VSync(0);
                /* The wait above allows 120 frames, but only 20 counts as a timeout. */
                if (attempts != 20)
                {
                    func_80032174(0, &status0, &status1);
                    switch (status1)
                    {
                    case 0:
                        g_carda_save_step++;
                        return phase_result;
                    case 1:
                        g_carda_entry_state = 0xFD;
                        g_carda_save_step = NULL;
                        return phase_result;
                    }
                }
            }
            if (g_carda_entry_state != 0xF6)
            {
                g_carda_entry_state = 0xF6;
            }
            g_carda_save_step = NULL;
            return phase_result;

        case CARDA_STEP_READ_SAVE_PREFIX:
            g_carda_progress_active = 1;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(&g_carda_selected_card_path, CARDA_FILE_ASYNC | CARDA_FILE_READ);
            carda_release_primary_handles();
            _card_wait(g_carda_card_slot);
            if (read(g_carda_file_handle, g_carda_save_blob, CARDA_SAVE_PREFIX_BYTES) == -1)
            {
                carda_open_save_status_dialog(1);
                return phase_result;
            }
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_SAVE_PREFIX_READ:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                g_carda_progress_active = 0;
                g_carda_save_step++;
                close(g_carda_file_handle);
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_progress_bar_active = 0;
                carda_open_save_status_dialog(1);
                return phase_result;
            }
            break;

        case CARDA_STEP_ARM_RETRIES:
            g_carda_retry_count = CARDA_SAVE_RETRIES;
            g_carda_save_step++;
            break;

        case CARDA_STEP_WRITE_TEMP_ALT_SAVE:
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
            strcat(&card_path, g_lom_alt_save_dummy_filename);
            _card_wait(g_carda_card_slot);
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
                    carda_open_save_status_dialog(0);
                    return phase_result;
                }
                break;
            }
            close(g_carda_file_handle);
            strcpy(g_carda_temp_card_path, &card_path);
            _card_wait(g_carda_card_slot);
            g_carda_file_handle = open(g_carda_temp_card_path, CARDA_FILE_ASYNC | CARDA_FILE_WRITE);
            carda_release_primary_handles();
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_bar_active = 1;
            _card_wait(g_carda_card_slot);
            if (write(g_carda_file_handle, g_carda_save_blob, CARDA_ALT_SAVE_BYTES) == -1)
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
                    carda_open_save_status_dialog(0);
                    return phase_result;
                }
                break;
            }
            g_carda_save_step++;
            break;

        case CARDA_STEP_POLL_TEMP_ALT_SAVE_WRITE:
            switch (carda_poll_primary_handle_group())
            {
            case CARDA_CARD_EVENT_READY:
                if (g_carda_preserve_old_save != 0)
                {
                    _card_wait(g_carda_card_slot);
                    for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(&g_carda_selected_card_path) != 0)
                        {
                            break;
                        }
                    }
                }
                _card_wait(g_carda_card_slot);
                for (attempts = 0; attempts < CARDA_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_carda_temp_card_path, &g_carda_selected_card_path) != 0)
                    {
                        break;
                    }
                }
                g_carda_save_in_progress = 0;
                /* Advanced, then replaced by the rescan table below. */
                g_carda_save_step++;
                close(g_carda_file_handle);
                g_carda_save_step = g_carda_steps_initial_scan;
                break;
            case CARDA_CARD_EVENT_ERROR:
            case CARDA_CARD_EVENT_TIMEOUT:
            case CARDA_CARD_EVENT_NEW_CARD:
                g_carda_retry_count--;
                if (g_carda_retry_count == 0)
                {
                    close(g_carda_file_handle);
                    g_carda_progress_bar_active = 0;
                    carda_open_save_status_dialog(0);
                    return phase_result;
                }
                /* Rewind to CARDA_STEP_WRITE_TEMP_ALT_SAVE and try again. */
                close(g_carda_file_handle);
                g_carda_save_step--;
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
    s32 attempt;
    s32 unused[4]; /* never used, but the original stack frame reserves it */

    for (attempt = 0; attempt < 20; attempt++)
    {
        if (_card_format(g_carda_card_slot << 4) != 0)
        {
            break;
        }
    }

    g_carda_preserve_old_save = 0;
    g_carda_selected_row = 0;
    strcpy(g_carda_entries[g_carda_card_slot], g_new_save_entry_prefix);
}

/**
 * @brief Clear the software card events, request card info and restart the sequence at g_carda_steps_idle.
 */
void carda_restart_card_sequence(void)
{
    _card_wait(g_carda_card_slot);
    carda_release_primary_handles();
    _card_info(g_carda_card_slot * 0x10);
    g_carda_save_step = g_carda_steps_idle;
}

/**
 * @brief Poll the software card events and request fresh card info after one fires.
 * @return Event index (0 done, 1 error, 2 timeout, 3 new card), or -1 if none is pending.
 */
s32 carda_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = carda_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        _card_wait(g_carda_card_slot);
        _card_info(g_carda_card_slot * 0x10);
    }
    return busy_slot;
}

/**
 * @brief Open and enable the software and hardware memory-card events.
 */
void carda_init_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    D_80166B94 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    D_80166B98 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    D_80166B9C = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    D_80166BA0 = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    D_80166108 = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    D_8016610C = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    D_80166110 = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    D_80166114 = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    EnableEvent(D_80166B94);
    EnableEvent(D_80166B98);
    EnableEvent(D_80166B9C);
    EnableEvent(D_80166BA0);
    EnableEvent(D_80166108);
    EnableEvent(D_8016610C);
    EnableEvent(D_80166110);
    EnableEvent(D_80166114);
    ExitCriticalSection();
    g_carda_progress_start_tick = VSync(-1);
    g_carda_progress_bar_active = 0;
    g_carda_entry_scan_active = 0;
}

/**
 * @brief Close all software and hardware memory-card events.
 */
void carda_shutdown_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(D_80166B94);
    CloseEvent(D_80166B98);
    CloseEvent(D_80166B9C);
    CloseEvent(D_80166BA0);
    CloseEvent(D_80166108);
    CloseEvent(D_8016610C);
    CloseEvent(D_80166110);
    CloseEvent(D_80166114);
    ExitCriticalSection();
}

/**
 * @brief Reset the list view and read the first directory entry of a card.
 * @param page Memory-card slot to scan.
 * @return 1 if the first entry was read or the mode allows an empty card, 0 otherwise.
 */
s32 carda_begin_entry_scan(s32 page)
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
    g_carda_entry_state = 0;
    for (attempt = 0; attempt < 20; attempt++)
    {
        if (firstfile(&search_path, g_carda_entries[page]) != 0)
        {
            func_800B0170(&g_carda_entries[page][g_carda_entry_state]);
            g_carda_entry_state += 1;
            return 1;
        }
    }
    if (g_carda_mode == 1 || g_carda_mode == 3)
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
s32 carda_scan_next_entry(s32 page)
{
    s32 entry_attempt;
    s32 selected;

    _card_wait(page);
    for (entry_attempt = 0; entry_attempt < 20; entry_attempt++)
    {
        if (nextfile(&g_carda_entries[page][g_carda_entry_state]) != 0)
        {
            func_800B0170(&g_carda_entries[page][g_carda_entry_state]);
            g_carda_entry_state += 1;
            return 1;
        }
    }

    field_reset_input_repeat();
    if (g_carda_mode == 1 && carda_has_known_entry_type() == 0)
    {
        g_carda_entry_state = 0xF8;
    }
    else
    {
        g_carda_preserve_old_save = 0;
        if (carda_card_lacks_free_blocks())
        {
            if (g_carda_mode == 0 || g_carda_mode == 2)
            {
                strcpy(&g_carda_entries[page][g_carda_entry_state], D_800ECFD0);
                g_carda_entries[page][g_carda_entry_state].size = 0;
                g_carda_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
            {
                if (g_carda_mode == 2 || g_carda_mode == 3)
                {
                    g_carda_entry_state = 0xF7;
                }
                else
                {
                    g_carda_entry_state = 0xFA;
                }
                D_80166AD8 = 0;
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
            if (g_carda_mode == 2 || g_carda_mode == 3)
            {
                strcpy(&g_carda_entries[page][g_carda_entry_state], g_new_save_entry_prefix);
                g_carda_entries[page][g_carda_entry_state].size = 0xC000;
                g_carda_entry_state += 1;
            }
            else if (g_carda_mode == 0)
            {
                strcpy(&g_carda_entries[page][g_carda_entry_state], g_new_save_entry_prefix);
                g_carda_entries[page][g_carda_entry_state].size = 0x4000;
                g_carda_entry_state += 1;
            }
            selected = carda_rank_entries();
            if (carda_has_known_entry_type() == 0)
            {
                g_carda_selected_row = 0;
                carda_scroll_to_selection();
                D_80166AD8 = 0;
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

    if (g_carda_entry_state == 0)
    {
        g_carda_selection_status = 3;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 8) == 0)
    {
        g_carda_selection_status = 2;
        return;
    }
    if (strncmp(D_800ECFD0, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 9) == 0)
    {
        g_carda_selection_status = 4;
        return;
    }
    memcpy(&card_path, &g_carda_card_path_prefix, 6);
    strcat(card_path.bytes, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name);
    card_path.bytes[2] += (u8)g_carda_card_slot;
    g_carda_selection_status = 0;
    strcpy(&g_carda_selected_card_path, card_path.bytes);
    g_carda_save_step = &g_carda_steps_read_selected_header[0];
    g_carda_io_busy = 1;
    if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 0xC) == 0)
    {
        g_carda_selected_entry_extended = 1;
    }
    else
    {
        g_carda_selected_entry_extended = 0;
    }
}

/**
 * @brief Consume pending software memory-card events.
 */
void carda_release_primary_handles(void)
{
    TestEvent(D_80166B94);
    TestEvent(D_80166B98);
    TestEvent(D_80166B9C);
    TestEvent(D_80166BA0);
}

/**
 * @brief Consume pending hardware memory-card events.
 */
void carda_release_secondary_handles(void)
{
    TestEvent(D_80166108);
    TestEvent(D_8016610C);
    TestEvent(D_80166110);
    TestEvent(D_80166114);
}

/**
 * @brief Consume the first pending software card event in priority order.
 * @return Event index (0 done, 1 error, 2 timeout, 3 new card), or -1 if none is pending.
 */
s32 carda_poll_primary_handle_group(void)
{
    if (TestEvent(D_80166B94) == 1)
    {
        return 0;
    }
    if (TestEvent(D_80166B98) == 1)
    {
        return 1;
    }
    if (TestEvent(D_80166B9C) == 1)
    {
        return 2;
    }
    if (TestEvent(D_80166BA0) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Consume the first pending hardware card event in priority order.
 * @return Event index (0 done, 1 error, 2 timeout, 3 new card), or -1 if none is pending.
 */
s32 carda_poll_secondary_handle_group(void)
{
    if (TestEvent(D_80166108) == 1)
    {
        return 0;
    }
    if (TestEvent(D_8016610C) == 1)
    {
        return 1;
    }
    if (TestEvent(D_80166110) == 1)
    {
        return 2;
    }
    if (TestEvent(D_80166114) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Reorder the current card's directory by save type and suffix byte.
 * @note g_lom_save_filename_prefix saves come first, then g_lom_alt_save_filename_prefix saves (each by suffix group),
 *       then the placeholder entries, then everything else.
 */
void carda_sort_entries_by_type(void)
{
    CardaDirEntry sorted_entries[CARDA_ENTRIES_PER_CARD];
    s32 output_count = 0;
    s32 group;
    s32 entry_index;

    for (group = 0; group < CARDA_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
        {
            if (g_carda_entry_suffix_values[entry_index] == group && strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
            {
                bcopy(&g_carda_entries[g_carda_card_slot][entry_index], &sorted_entries[output_count], sizeof(CardaDirEntry));
                output_count++;
            }
        }
    }

    for (group = 0; group < CARDA_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
        {
            if (g_carda_entry_suffix_values[entry_index] == group && strncmp(g_lom_alt_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) == 0)
            {
                bcopy(&g_carda_entries[g_carda_card_slot][entry_index], &sorted_entries[output_count], sizeof(CardaDirEntry));
                output_count++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 8) == 0 ||
            strncmp(D_800ECFD0, g_carda_entries[g_carda_card_slot][entry_index].name, 9) == 0)
        {
            bcopy(&g_carda_entries[g_carda_card_slot][entry_index], &sorted_entries[output_count], sizeof(CardaDirEntry));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) != 0 &&
            strncmp(g_lom_alt_save_filename_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 0xC) != 0 &&
            strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][entry_index].name, 8) != 0 &&
            strncmp(D_800ECFD0, g_carda_entries[g_carda_card_slot][entry_index].name, 9) != 0)
        {
            bcopy(&g_carda_entries[g_carda_card_slot][entry_index], &sorted_entries[output_count], sizeof(CardaDirEntry));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_carda_entry_state; entry_index++)
    {
        bcopy(&sorted_entries[entry_index], &g_carda_entries[g_carda_card_slot][entry_index], sizeof(CardaDirEntry));
    }
}
