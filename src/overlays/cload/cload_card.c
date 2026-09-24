#include "cload_internal.h"

/**
 * @brief Load-sequence step opcodes stored in the g_cload_steps_* byte tables.
 * @note Opcodes without a case (7, 10-14, 21-23, 25-29) are no-ops; 14 is the
 *       idle opcode that g_cload_steps_idle parks on.
 */
typedef enum CloadLoadStep
{
    CLOAD_STEP_DONE = 0,               /**< End of a step table; report phase 2. */
    CLOAD_STEP_CARD_INFO = 1,          /**< Issue _card_info on the current slot. */
    CLOAD_STEP_POLL_CARD_INFO = 2,     /**< Wait for the _card_info result. */
    CLOAD_STEP_RELEASE_PRIMARY = 3,    /**< Clear the primary card event group. */
    CLOAD_STEP_WAIT_SECONDARY = 4,     /**< Wait for and check the secondary card events. */
    CLOAD_STEP_RELEASE_SECONDARY = 5,  /**< Clear the secondary card event group. */
    CLOAD_STEP_SCAN_ENTRIES = 6,       /**< Erase the dummy files and scan the card directory. */
    CLOAD_STEP_CARD_CLEAR = 8,         /**< Issue _card_clear on the current slot. */
    CLOAD_STEP_CARD_LOAD = 9,          /**< Issue _card_load and arm the poll countdowns. */
    CLOAD_STEP_IDLE = 14,              /**< No-op step that never advances. */
    CLOAD_STEP_POLL_CARD_LOAD = 15,    /**< Wait for the _card_clear/_card_load result, retrying. */
    CLOAD_STEP_DRAIN_SECONDARY = 16,   /**< Wait for any secondary card event. */
    CLOAD_STEP_READ_HEADER = 17,       /**< Open the selected save and start reading its header. */
    CLOAD_STEP_POLL_HEADER_READ = 18,  /**< Wait for the header read to finish. */
    CLOAD_STEP_READ_SAVE = 19,         /**< Open the selected save and start reading the blob. */
    CLOAD_STEP_POLL_SAVE_READ = 20,    /**< Wait for the blob read to finish, retrying. */
    CLOAD_STEP_CHECK_CARD_TYPE = 24,   /**< Wait for a card and check its status. */
    CLOAD_STEP_ARM_SAVE_RETRIES = 30   /**< Arm the save read retry counter. */
} CloadLoadStep;

/**
 * @brief Result index returned by the card event group pollers.
 * @note Matches the order of the SwCARD IOE/ERROR/TIMEOUT/NEWCARD events.
 */
typedef enum CloadCardEvent
{
    CLOAD_CARD_EVENT_READY = 0,     /**< Operation completed. */
    CLOAD_CARD_EVENT_ERROR = 1,     /**< Card reported an error. */
    CLOAD_CARD_EVENT_TIMEOUT = 2,   /**< No card or no response. */
    CLOAD_CARD_EVENT_NEW_CARD = 3   /**< A different card was inserted. */
} CloadCardEvent;

/**
 * @brief Phase codes returned by cload_advance_load_sequence.
 * @see cload_update_load_sequence
 */
typedef enum CloadLoadResult
{
    CLOAD_LOAD_CONTINUE = 1,      /**< Step handled; poll again next frame. */
    CLOAD_LOAD_FINISHED = 2,      /**< Step table ended; caller arms card_reset. */
    CLOAD_LOAD_REPEAT = 3,        /**< Card command issued; run the next step now. */
    CLOAD_LOAD_REFRESH = 4,       /**< Card missing or failed; caller arms refresh_entries. */
    CLOAD_LOAD_CARD_CHANGED = 5   /**< Card replaced during load; caller reports 0xF9. */
} CloadLoadResult;

/**
 * @brief Report "no memory card" and ask the caller to refresh the entry list.
 * @param result Phase-result variable that receives CLOAD_LOAD_REFRESH.
 */
#define CLOAD_REQUEST_REFRESH(result)          \
    do                                         \
    {                                          \
        (result) = CLOAD_LOAD_REFRESH;         \
        g_cload_selection_status = 0;          \
        g_cload_entry_state = 0xFD;            \
    } while (0)

/**
 * @brief Finish the current step table with a result, an entry status and a next table.
 * @param result Phase-result variable that receives @p code.
 * @param code CloadLoadResult to return to the caller.
 * @param entry_state New g_cload_entry_state status code.
 * @param next_step New g_cload_load_step table (NULL stops the sequence).
 */
#define CLOAD_END_STEP_TABLE(result, code, entry_state, next_step) \
    do                                                             \
    {                                                              \
        (result) = (code);                                         \
        g_cload_entry_state = (entry_state);                       \
        g_cload_load_step = (next_step);                           \
    } while (0)

/** @brief Memory-card device path prefix for card files. */
const CloadCardPathTemplate g_cload_card_path_prefix = {"bu00:"};

/**
 * @brief Validate a loaded save blob against its trailing checksum and magic.
 * @param blob Loaded save blob; its payload is summed by cload_compute_save_checksum.
 * @return 1 if the stored checksum matches and the magic equals CLOAD_SAVE_MAGIC, otherwise 0.
 */
s32 cload_validate_save_blob(CloadSaveBlob *blob)
{
    if (blob->checksum == cload_compute_save_checksum(blob->payload))
    {
        if (blob->magic == CLOAD_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}


/**
 * @brief Compute the additive checksum used to validate a loaded save payload.
 * @param data Start of the CLOAD_SAVE_PAYLOAD_BYTES-byte save payload.
 * @return Twice the byte sum plus CLOAD_SAVE_CHECKSUM_BIAS.
 */
s32 cload_compute_save_checksum(u8 *data)
{
    s32 sum;
    u32 byte_index;
    u8 *cursor;

    cursor = data;
    sum = 0;
    byte_index = 0;
    do
    {
        byte_index++;
        sum += *cursor;
        cursor++;
    } while (byte_index < CLOAD_SAVE_PAYLOAD_BYTES);
    return sum * 2 + CLOAD_SAVE_CHECKSUM_BIAS;
}


/**
 * @brief Render the hex nibbles of @p value to ASCII, suppressing leading zeros.
 * @param out Destination buffer; receives the ASCII digits and a terminating 0.
 * @param value Value whose nibbles (most-significant first) are emitted.
 * @param max_chars Maximum number of characters to emit.
 * @note Each nibble is converted by cload_hex_nibble_to_ascii; a leading run of zero nibbles
 *       is skipped until the first non-zero digit is seen.
 */
void cload_format_hex(s8 *out, s32 value, s32 max_chars)
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
                cload_hex_nibble_to_ascii(out, nibble);
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
 * @brief Convert a 4-bit value to its ASCII hex digit and store it.
 *
 * Writes '0'-'9' for @p nibble 0-9, 'A'-'F' for 10-15, and '_' (0x5F) for any
 * value >= 16, storing the single character byte at @p out.
 *
 * @param out Destination byte written with the ASCII character.
 * @param nibble Value to convert; expected range 0-15.
 */
void cload_hex_nibble_to_ascii(s8 *out, s32 nibble)
{
    if (nibble < 0xA)
    {
        *out = nibble + 0x30;
        return;
    }
    if (nibble < 0x10)
    {
        *out = nibble + 0x37;
        return;
    }
    *out = 0x5F;
}


/**
 * @brief Parse an ASCII hex string into a 32-bit value.
 * @param text Pointer to the hex text (0-9, A-F, a-f).
 * @param digits_left Maximum number of characters to consume.
 * @return The accumulated big-endian value of the hex digits read.
 */
u32 cload_parse_hex(u8 *text, s32 digits_left)
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
 * @brief Skip a leading run of hex characters and one separator, then parse up
 *        to two hex digits into an integer value.
 * @param text Pointer to the ASCII text to scan.
 * @return The value of the (at most two) hex digits found after the separator.
 */
s32 cload_parse_hex_suffix_byte(u8 *text)
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
 * @brief Parse the hex-string field of each recognized directory entry and record the results.
 *
 * Entries whose name starts with g_lom_save_filename_prefix have up to five hex
 * digits at name offset 0xC parsed into g_cload_entry_fields; the suffix byte
 * after that run is parsed by @ref cload_parse_hex_suffix_byte into
 * g_cload_entry_suffix_values. Unrecognized entries store -1 / 0 instead.
 *
 * @return The largest suffix byte among the recognized entries (0 if none).
 */
s32 cload_parse_entry_fields(void)
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
    while (entry_index < g_cload_entry_state)
    {
        if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) == 0)
        {
            digits_left = 5;
            cursor = (u8 *)(g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES + entry_index * CLOAD_DIRECTORY_ENTRY_BYTES + (s32)g_cload_entries + 0xC);
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
            suffix = (u8 *)&CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name[0xC];
            g_cload_entry_fields[g_cload_card_slot][entry_index] = value;
            suffix_value = cload_parse_hex_suffix_byte(suffix);
            g_cload_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            g_cload_entry_fields[g_cload_card_slot][entry_index] = -1;
            g_cload_entry_suffix_values[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}


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

    cload_parse_entry_fields();
    maximum = -1;
    cload_sort_entries_by_type();
    max_suffix = cload_parse_entry_fields();
    cload_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (g_cload_entry_fields[g_cload_card_slot][entry_index] >= 0)
        {
            if (g_cload_entry_fields[g_cload_card_slot][entry_index] >= maximum)
            {
                g_cload_entry_ranks[entry_index] = next_rank;
                maximum = g_cload_entry_fields[g_cload_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_cload_entry_fields[g_cload_card_slot][entry_index] < g_cload_entry_fields[g_cload_card_slot][previous_index])
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
    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (next_rank < g_cload_entry_fields[g_cload_card_slot][entry_index])
        {
            next_rank = g_cload_entry_fields[g_cload_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    g_cload_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 8) == 0)
        {
            g_cload_entry_suffix_values[entry_index] = max_suffix + 1;
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
 * @brief Scan up to g_cload_entry_state entries of the g_cload_entries table (row
 *        selected by g_cload_card_slot, stride 0x28) and report whether any entry
 *        matches one of the two known-type patterns g_lom_save_filename_prefix / g_lom_alt_save_filename_prefix.
 * @return 1 on the first entry that matches either pattern (strncmp returns 0
 *         on a match), 0 if no entry matches.
 */
s32 cload_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) == 0 ||
            strncmp(g_lom_alt_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) == 0)
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
    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        used_blocks += CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).size / CLOAD_MEMORY_CARD_BLOCK_BYTES;
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
    CloadCardPathScratch card_path;

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_cload_card_slot;
    strcat(card_path.bytes, g_lom_save_dummy_filename);
    erase(&card_path);

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    card_path.bytes[2] += (u8)g_cload_card_slot;
    strcat(card_path.bytes, g_lom_alt_save_dummy_filename);
    erase(&card_path);
}


/**
 * @brief Run the current memory-card load step and advance g_cload_load_step.
 * @return CloadLoadResult phase code for cload_update_load_sequence.
 * @note g_cload_load_step walks one of the g_cload_steps_* byte tables; each
 *       CloadLoadStep opcode issues or polls a card command, reads the selected
 *       save, or scans the card directory, and updates g_cload_entry_state /
 *       g_cload_selection_status. Opcodes with no case are no-ops.
 */
s32 cload_advance_load_sequence(void)
{
    CloadLoadScratch card_path;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 scan_attempts;
    s32 wait_attempts;
    s32 rank_index;
    s32 rank_fill;
    s32 poll_result;

    /* Builds the "bu00:" slot path like the erase helper; never used afterwards. */
    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    phase_result = CLOAD_LOAD_CONTINUE;
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;

    if (g_cload_load_step != NULL)
    {
        switch (*g_cload_load_step)
        {
        case CLOAD_STEP_CARD_INFO:
            phase_result = CLOAD_LOAD_REPEAT;
            _card_wait(g_cload_card_slot);
            _card_info(g_cload_card_slot * 0x10);
            g_cload_load_step++;
            break;

        case CLOAD_STEP_POLL_CARD_INFO:
            switch (cload_poll_primary_handle_group())
            {
            case CLOAD_CARD_EVENT_READY:
                g_cload_load_step++;
                break;
            case CLOAD_CARD_EVENT_ERROR:
            case CLOAD_CARD_EVENT_TIMEOUT:
                phase_result = CLOAD_LOAD_REFRESH;
                g_cload_selection_status = 0;
                g_cload_entry_state = 0xFD;
                g_cload_load_step++;
                cload_deactivate_primary_element();
                break;
            case CLOAD_CARD_EVENT_NEW_CARD:
                g_cload_rank_count = 0x28;
                rank_fill = -1;
                for (rank_index = 14; rank_index >= 0; rank_index--)
                {
                    g_cload_entry_ranks[rank_index] = rank_fill;
                }
                g_cload_entry_state = 0xFF;
                g_cload_load_step = g_cload_steps_initial_scan;
                break;
            }
            break;

        case CLOAD_STEP_RELEASE_PRIMARY:
            cload_release_primary_handles();
            g_cload_load_step++;
            break;

        case CLOAD_STEP_WAIT_SECONDARY:
            do
            {
                poll_result = cload_poll_secondary_handle_group();
            } while (poll_result == -1);
            switch (poll_result)
            {
            case CLOAD_CARD_EVENT_READY:
                g_cload_load_step++;
                break;
            case CLOAD_CARD_EVENT_ERROR:
            case CLOAD_CARD_EVENT_TIMEOUT:
            case CLOAD_CARD_EVENT_NEW_CARD:
                CLOAD_REQUEST_REFRESH(phase_result);
                break;
            }
            break;

        case CLOAD_STEP_RELEASE_SECONDARY:
            cload_release_secondary_handles();
            g_cload_load_step++;
            break;

        case CLOAD_STEP_SCAN_ENTRIES:
            cload_erase_fixed_card_files();
            g_cload_entry_scan_active = 1;
            if (cload_begin_entry_scan(g_cload_card_slot) == 0)
            {
                CLOAD_END_STEP_TABLE(phase_result, CLOAD_LOAD_FINISHED, 0xF8, NULL);
                g_cload_entry_scan_active = 0;
                break;
            }
            scan_attempts = 0;
            g_cload_load_step++;
            do
            {
                if (cload_scan_next_entry(g_cload_card_slot) == 0)
                {
                    g_cload_entry_scan_active = 0;
                    if (g_cload_entry_state != 0xF8 && g_cload_entry_state != 0xFA)
                    {
                        cload_commit_selected_entry();
                    }
                    break;
                }
                scan_attempts++;
            } while (scan_attempts < 0x14);
            break;

        case CLOAD_STEP_CARD_CLEAR:
            phase_result = CLOAD_LOAD_REPEAT;
            _card_wait(g_cload_card_slot);
            _card_clear(g_cload_card_slot * 0x10);
            g_cload_load_step++;
            break;

        case CLOAD_STEP_CARD_LOAD:
            phase_result = CLOAD_LOAD_REPEAT;
            _card_wait(g_cload_card_slot);
            _card_load(g_cload_card_slot * 0x10);
            g_cload_primary_poll_countdown = 0x10;
            g_cload_secondary_poll_countdown = 0x10;
            g_cload_load_step++;
            break;

        case CLOAD_STEP_DONE:
            phase_result = CLOAD_LOAD_FINISHED;
            D_80162370 = 0;
            break;

        case CLOAD_STEP_POLL_CARD_LOAD:
            switch (cload_poll_primary_handle_group())
            {
            case CLOAD_CARD_EVENT_READY:
                g_cload_load_step++;
                break;
            case CLOAD_CARD_EVENT_ERROR:
            case CLOAD_CARD_EVENT_TIMEOUT:
                g_cload_secondary_poll_countdown--;
                if (g_cload_secondary_poll_countdown == 0)
                {
                    CLOAD_REQUEST_REFRESH(phase_result);
                }
                else
                {
                    _card_wait(g_cload_card_slot);
                    _card_clear(g_cload_card_slot * 0x10);
                    _card_wait(g_cload_card_slot);
                    _card_load(g_cload_card_slot * 0x10);
                }
                break;
            case CLOAD_CARD_EVENT_NEW_CARD:
                g_cload_primary_poll_countdown--;
                if (g_cload_primary_poll_countdown != 0)
                {
                    _card_wait(g_cload_card_slot);
                    _card_clear(g_cload_card_slot * 0x10);
                    _card_wait(g_cload_card_slot);
                    _card_load(g_cload_card_slot * 0x10);
                }
                else
                {
                    CLOAD_END_STEP_TABLE(phase_result, CLOAD_LOAD_CARD_CHANGED, 0xFC,
                                         g_cload_steps_idle);
                }
                break;
            }
            break;

        case CLOAD_STEP_DRAIN_SECONDARY:
            do
            {
                poll_result = cload_poll_secondary_handle_group();
            } while (poll_result == -1);
            g_cload_load_step++;
            break;

        case CLOAD_STEP_READ_HEADER:
            g_cload_io_busy = 1;
            g_cload_selection_status = 0;
            _card_wait(g_cload_card_slot);
            g_cload_file_handle = open(g_cload_selected_card_path, 0x8001);
            if (g_cload_file_handle == -1)
            {
                break;
            }
            cload_release_primary_handles();
            _card_wait(g_cload_card_slot);
            if (read(g_cload_file_handle, g_cload_selected_file_header,
                     g_cload_selected_entry_extended != 0 ? 0x280 : 0x80) != -1)
            {
                g_cload_load_step++;
            }
            else
            {
                close(g_cload_file_handle);
            }
            break;

        case CLOAD_STEP_POLL_HEADER_READ:
            poll_result = cload_poll_primary_handle_group();
            if (poll_result == CLOAD_CARD_EVENT_READY)
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
                g_cload_entry_state = 0xFF;
                g_cload_load_step = g_cload_steps_initial_scan;
            }
            break;

        case CLOAD_STEP_ARM_SAVE_RETRIES:
            g_cload_retry_count = 5;
            g_cload_load_step++;
            break;

        case CLOAD_STEP_READ_SAVE:
            g_cload_progress_active = 1;
            g_cload_progress_bar_active = 1;
            g_cload_progress_start_tick = VSync(-1);
            _card_wait(g_cload_card_slot);
            g_cload_file_handle = open(g_cload_selected_card_path, 0x8001);
            cload_release_primary_handles();
            _card_wait(g_cload_card_slot);
            if (read(g_cload_file_handle, &g_cload_save_blob, 0x4000) != -1)
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

        case CLOAD_STEP_POLL_SAVE_READ:
            switch (cload_poll_primary_handle_group())
            {
            case CLOAD_CARD_EVENT_READY:
                g_cload_progress_active = 0;
                g_cload_load_step++;
                close(g_cload_file_handle);
                break;
            case CLOAD_CARD_EVENT_ERROR:
            case CLOAD_CARD_EVENT_TIMEOUT:
            case CLOAD_CARD_EVENT_NEW_CARD:
                g_cload_retry_count--;
                if (g_cload_retry_count != 0)
                {
                    /* Rewind to CLOAD_STEP_READ_SAVE and try again. */
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

        case CLOAD_STEP_CHECK_CARD_TYPE:
            for (wait_attempts = 0; wait_attempts < 0x14; wait_attempts++)
            {
                if (McxCardType(g_cload_card_slot * 0x10) == 1)
                {
                    break;
                }
                VSync(0);
            }
            if (wait_attempts != 0x14)
            {
                func_80032174(0, &status0, &status1);
                if (status1 == 0)
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

const CloadCardPathTemplate g_cload_card_search_path = {"bu00:*"};


/**
 * @brief Reset the cached resource handles and arm the first load step.
 * @note Releases the handles (cload_release_primary_handles), rewinds the CD channel, and points
 *       g_cload_load_step at the g_cload_steps_idle step table.
 */
void cload_restart_load_sequence(void)
{
    cload_release_primary_handles();
    _card_wait(g_cload_card_slot);
    _card_info(g_cload_card_slot * 0x10);
    g_cload_load_step = g_cload_steps_idle;
}


/**
 * @brief Poll the four cached handles; on completion, rewind the CD channel.
 * @return The busy-slot index from cload_poll_primary_handle_group (-1 when none are busy).
 */
s32 cload_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = cload_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        _card_wait(g_cload_card_slot);
        _card_info(g_cload_card_slot * 0x10);
    }
    return busy_slot;
}


/**
 * @brief Allocate and register the eight streaming buffers for this overlay.
 * @note Brackets the eight OpenEvent allocations (handles stored in
 *       g_cload_primary_handle0..g_cload_secondary_handle3) with EnterCriticalSection / ExitCriticalSection and resets the
 *       stream bookkeeping (g_cload_entry_scan_active, g_cload_progress_start_tick, g_cload_progress_bar_active).
 */
void cload_init_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_cload_primary_handle0 = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_cload_primary_handle1 = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_cload_primary_handle2 = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_cload_primary_handle3 = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_cload_secondary_handle0 = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_cload_secondary_handle1 = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_cload_secondary_handle2 = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_cload_secondary_handle3 = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    EnableEvent(g_cload_primary_handle0);
    EnableEvent(g_cload_primary_handle1);
    EnableEvent(g_cload_primary_handle2);
    EnableEvent(g_cload_primary_handle3);
    EnableEvent(g_cload_secondary_handle0);
    EnableEvent(g_cload_secondary_handle1);
    EnableEvent(g_cload_secondary_handle2);
    EnableEvent(g_cload_secondary_handle3);
    ExitCriticalSection();
    g_cload_entry_scan_active = 0;
    g_cload_progress_start_tick = VSync(-1);
    g_cload_progress_bar_active = 0;
}


/**
 * @brief Tear down / release the eight g_cload_primary_handle0..g_cload_secondary_handle3 handles.
 * @note Wrapped by reset_controller_vsync_state and EnterCriticalSection/ExitCriticalSection bracket calls;
 *       each handle is passed to CloseEvent in turn (g_cload_entry_scan_active is skipped).
 */
void cload_shutdown_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_cload_primary_handle0);
    CloseEvent(g_cload_primary_handle1);
    CloseEvent(g_cload_primary_handle2);
    CloseEvent(g_cload_primary_handle3);
    CloseEvent(g_cload_secondary_handle0);
    CloseEvent(g_cload_secondary_handle1);
    CloseEvent(g_cload_secondary_handle2);
    CloseEvent(g_cload_secondary_handle3);
    ExitCriticalSection();
}


/**
 * @brief Begin streaming the page's first g_cload_entries record.
 * @param page Page index (each page is 0x320 bytes in g_cload_entries).
 * @return 1 if firstfile accepted the record (count bumped), else 0.
 */
s32 cload_begin_entry_scan(s32 page)
{
    CloadCardSearchPathBuffer search_path;

    memcpy(&search_path, &g_cload_card_search_path, 7);
    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_cload_entry_state = 0;
    search_path.bytes[2] += page;
    if (firstfile(&search_path, &CLOAD_DIR_ENTRY(page, 0)) != 0)
    {
        g_cload_entry_state += 1;
        return 1;
    }
    return 0;
}


/**
 * @brief Try to append the page's next g_cload_entries record; if it cannot,
 *        recompute the page's fixed-point total and update the selection state.
 * @param page Page index (each page is 0x320 bytes / 20 records in g_cload_entries).
 * @return 1 if nextfile accepted the new record (count bumped), else 0.
 * @note When the directory is complete, cload_entry_blocks_reach_limit decides
 *       whether a full card clamps the state (0xFA) or the selection
 *       (g_cload_selected_row) is set from cload_rank_entries's result.
 */
s32 cload_scan_next_entry(s32 page)
{
    s32 selected;

    if (nextfile(&CLOAD_DIR_ENTRY(page, g_cload_entry_state)) != 0)
    {
        g_cload_entry_state += 1;
        return 1;
    }
    field_reset_input_repeat();
    if (cload_has_known_entry_type() == 0)
    {
        g_cload_entry_state = 0xF8;
    }
    else
    {
        if (cload_entry_blocks_reach_limit())
        {
            selected = cload_rank_entries();
            if (cload_has_known_entry_type() == 0)
            {
                g_cload_entry_state = 0xFA;
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
 * @brief Commit the selected g_cload_entries record and arm the next step.
 * @note Rejects the new-save placeholder entry, builds "buX0:<name>" for the
 *       selected entry, copies it to g_cload_selected_card_path, and flags
 *       whether the entry is an extended (g_lom_save_filename_prefix) save.
 */
void cload_commit_selected_entry(void)
{
    CloadCardPathBuffer card_path;

    if (g_cload_entry_state == 0)
    {
        g_cload_selection_status = 3;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, g_cload_selected_row).name, 8) == 0)
    {
        g_cload_selection_status = 2;
        return;
    }
    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    strcat(card_path.bytes, CLOAD_DIR_ENTRY(g_cload_card_slot, g_cload_selected_row).name);
    card_path.bytes[2] += (u8)g_cload_card_slot;
    g_cload_selection_status = 0;
    strcpy(g_cload_selected_card_path, card_path.bytes);
    g_cload_load_step = &g_cload_steps_read_selected_header[0];
    if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, g_cload_selected_row).name, 0xC) == 0)
    {
        g_cload_selected_entry_extended = 1;
        return;
    }
    g_cload_selected_entry_extended = 0;
}


/**
 * @brief Release the four cached resource handles for this overlay.
 *
 * Passes the values held in g_cload_primary_handle0, g_cload_primary_handle1, g_cload_primary_handle2, and g_cload_primary_handle3
 * (in that order) to @ref TestEvent.
 */
void cload_release_primary_handles(void)
{
    TestEvent(g_cload_primary_handle0);
    TestEvent(g_cload_primary_handle1);
    TestEvent(g_cload_primary_handle2);
    TestEvent(g_cload_primary_handle3);
}


/**
 * @brief Release the next four cached resource handles for this overlay.
 *
 * Passes the values held in g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, and g_cload_secondary_handle3
 * (in that order) to @ref TestEvent.
 */
void cload_release_secondary_handles(void)
{
    TestEvent(g_cload_secondary_handle0);
    TestEvent(g_cload_secondary_handle1);
    TestEvent(g_cload_secondary_handle2);
    TestEvent(g_cload_secondary_handle3);
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_primary_handle0, g_cload_primary_handle1, g_cload_primary_handle2, g_cload_primary_handle3 to
 * @ref TestEvent in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 */
s32 cload_poll_primary_handle_group(void)
{
    if (TestEvent(g_cload_primary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_cload_primary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_cload_primary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_cload_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, g_cload_secondary_handle3 to
 * @ref TestEvent in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 */
s32 cload_poll_secondary_handle_group(void)
{
    if (TestEvent(g_cload_secondary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_cload_secondary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_cload_secondary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_cload_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Collate the g_cload_entries page records, ordering them by pattern class.
 * @note Five passes bucket records matching g_lom_save_filename_prefix, then g_lom_alt_save_filename_prefix, then
 *       g_new_save_entry_prefix, then the remainder, copying each 0x28-byte record with
 *       bcopy before writing the ordered set back to the page.
 */
void cload_sort_entries_by_type(void)
{
    CloadDirEntry sorted_entries[CLOAD_ENTRIES_PER_CARD];
    s32 output_count = 0;
    s32 group;
    s32 entry_index;

    for (group = 0; group < CLOAD_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
        {
            if (g_cload_entry_suffix_values[entry_index] == group &&
                strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) == 0)
            {
                bcopy(&CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index), &sorted_entries[output_count], sizeof(CloadDirEntry));
                output_count++;
            }
        }
    }

    for (group = 0; group < CLOAD_ENTRY_GROUP_COUNT; group++)
    {
        for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
        {
            if (g_cload_entry_suffix_values[entry_index] == group &&
                strncmp(g_lom_alt_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) == 0)
            {
                bcopy(&CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index), &sorted_entries[output_count], sizeof(CloadDirEntry));
                output_count++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 8) == 0)
        {
            bcopy(&CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index), &sorted_entries[output_count], sizeof(CloadDirEntry));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) != 0 &&
            strncmp(g_lom_alt_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 0xC) != 0 &&
            strncmp(g_new_save_entry_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index).name, 8) != 0)
        {
            bcopy(&CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index), &sorted_entries[output_count], sizeof(CloadDirEntry));
            output_count++;
        }
    }

    for (entry_index = 0; entry_index < g_cload_entry_state; entry_index++)
    {
        bcopy(&sorted_entries[entry_index], &CLOAD_DIR_ENTRY(g_cload_card_slot, entry_index), sizeof(CloadDirEntry));
    }
}
