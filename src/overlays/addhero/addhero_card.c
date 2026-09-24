#include "field_text.h"
#include "addhero_internal.h"

/**
 * @brief Address of the hex field after the save prefix of directory entry @p index on card @p card.
 * @note Summed as integers, offsets first, like the original code.
 */
#define ADDHERO_ENTRY_FIELD_TEXT(card, index) \
    ((u8*)((card) * ADDHERO_CARD_DIRECTORY_BYTES + (index) * ADDHERO_DIRECTORY_ENTRY_BYTES + (s32)g_addhero_entries + ADDHERO_SAVE_FILENAME_PREFIX_LENGTH))

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

/** @brief Size of the save blob (two memory-card blocks). */
#define ADDHERO_SAVE_BYTES 0x4000

/** @brief Attempts made at a synchronous card file operation before giving up. */
#define ADDHERO_FILE_OP_ATTEMPTS 20

/** @brief Retries of a failed asynchronous save read or write (ADDHERO_STEP_INIT_RETRIES). */
#define ADDHERO_SAVE_RETRIES 5

/** @brief Card-path workspace retained while advancing a load/save sequence. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[100];
} AddheroSequenceFilePath;

/**
 * @brief Commands in the card load/save sequence bytecode.
 * @note Opcodes without a case are no-ops.
 */
typedef enum
{
    ADDHERO_STEP_DONE = 0,                   /**< End of a step table; report ADDHERO_LOAD_RESULT_ABORT. */
    ADDHERO_STEP_CARD_INFO = 1,              /**< Issue _card_info on the current slot. */
    ADDHERO_STEP_POLL_CARD_INFO = 2,         /**< Wait for the _card_info result. */
    ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS = 3,  /**< Clear the software card events. */
    ADDHERO_STEP_POLL_HARDWARE_EVENTS = 4,   /**< Wait for and check the hardware card events. */
    ADDHERO_STEP_CLEAR_HARDWARE_EVENTS = 5,  /**< Clear the hardware card events. */
    ADDHERO_STEP_SCAN_ENTRIES = 6,           /**< Erase the placeholder files and scan the card directory. */
    ADDHERO_STEP_CLEAR_CARD = 8,             /**< Issue _card_clear on the current slot. */
    ADDHERO_STEP_LOAD_CARD = 9,              /**< Issue _card_load and arm the poll countdowns. */
    ADDHERO_STEP_ERASE_ENTRY = 10,           /**< Erase the selected directory entry. */
    ADDHERO_STEP_POLL_CARD_LOAD = 15,        /**< Wait for the _card_clear/_card_load result, retrying. */
    ADDHERO_STEP_WAIT_HARDWARE_EVENTS = 16,  /**< Wait for any hardware card event. */
    ADDHERO_STEP_READ_ENTRY = 17,            /**< Open the selected save and start reading its header. */
    ADDHERO_STEP_POLL_ENTRY_READ = 18,       /**< Wait for the header read to finish. */
    ADDHERO_STEP_READ_SAVE = 19,             /**< Open the selected save and start reading the blob. */
    ADDHERO_STEP_POLL_SAVE_READ = 20,        /**< Wait for the blob read to finish, retrying. */
    ADDHERO_STEP_CHECK_CARD_TYPE = 24,       /**< Wait for a card and check its status. */
    ADDHERO_STEP_WRITE_SAVE = 25,            /**< Create the placeholder file and start writing the save blob. */
    ADDHERO_STEP_POLL_SAVE_WRITE = 26,       /**< Wait for the write and rename it over the selected save. */
    ADDHERO_STEP_READ_BEFORE_WRITE = 27,     /**< Open the selected save and read the blob before writing. */
    ADDHERO_STEP_POLL_PREWRITE_READ = 28,    /**< Wait for that read to finish, retrying. */
    ADDHERO_STEP_INIT_RETRIES = 30           /**< Arm the read/write retry counter. */
} AddheroCardStep;

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

extern s32 g_addhero_retry_count;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_file_handle;
extern u8 g_addhero_target_file_path[];
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

s32 open(void* a, s32 b);
s32 rename(void* a, void* b);
s32 _card_load(s32 a);
s32 _card_clear(s32 a);
s32 func_80032174(s32 a, void* b, s32* c);
s32 McxCardType(s32 a);
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
 * @brief Format @p value as a big-endian double-byte decimal glyph string,
 *        suppressing leading zeros; emits a fixed overflow string past 999999.
 * @param out Destination glyph buffer.
 * @param value Value to format.
 * @return Pointer to the terminator written after the last glyph.
 */
s8* addhero_format_decimal(s8* out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8* p;

    p = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(AddheroOverflowGlyphString*)p = g_addhero_decimal_overflow_glyphs;
        return p + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + 0x824F) >> 8;
            *p++ = digit + 0x4F;
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
    *p = 0;
    return p;
}

/**
 * @brief Format @p value as an ASCII hex string of up to @p max_chars digits,
 *        suppressing leading zeros, and null-terminate it.
 * @param out       Destination character buffer.
 * @param value     Value to format.
 * @param max_chars Maximum number of hex digits to emit.
 */
void addhero_format_hex(s8* out, s32 value, s32 max_chars)
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
                addhero_hex_nibble_to_ascii(out, nibble);
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
 * @brief Write one nibble as its ASCII hex digit ('0'-'9', 'A'-'F'), or '_' for
 *        out-of-range values.
 * @param out   Destination byte.
 * @param value Nibble value to convert.
 * @see decomp.me (100%)
 */
void addhero_hex_nibble_to_ascii(s8* out, s32 value)
{
    if (value < 10)
    {
        *out = value + '0';
    }
    else if (value < 16)
    {
        *out = value + ('A' - 10);
    }
    else
    {
        *out = '_';
    }
}

/**
 * @brief Parse up to @p len leading hex digits from @p s into an integer.
 * @param s   Text to parse.
 * @param len Maximum number of hex digits to consume.
 * @return The parsed value; 0 when no hex digits are present.
 * @see decomp.me (100%)
 */
u32 addhero_parse_hex(u8* s, s32 len)
{
    u32 result;

    result = 0;
    while (((u8)(*s - '0') < 10) || ((u8)(*s - 'a') < 6) || ((u8)(*s - 'A') < 6))
    {
        if (len == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*s - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *s;
        }
        s++;
        len--;
    }
    return result;
}

/**
 * @brief Skip the leading hex-digit run of a field, then parse the next two hex
 *        digits (the suffix byte) that follow it.
 * @param text Field text to scan.
 * @return The parsed two-digit suffix byte value.
 * @see decomp.me (100%)
 */
s32 addhero_parse_hex_suffix_byte(u8* text)
{
    s32 count;
    u32 result;

    while ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'f') || (*text >= 'A' && *text <= 'F'))
    {
        text++;
    }
    text++;
    count = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (count == 0)
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
        count--;
    }
    return result;
}

/**
 * @brief Parse the hex value suffix of every "SD"-tagged directory entry on the
 *        active card, recording per-entry field values and their ranked bytes.
 * @return The maximum suffix byte value seen across all matching entries.
 * @see decomp.me (100%) https://decomp.me/scratch/7hY8R
 */
s32 addhero_parse_entry_fields(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8* cursor;
    u8* suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;
    s32 suffix_value;

    entry_index = 0;
    max_suffix = entry_index;
    while (entry_index < g_addhero_entry_state)
    {
        if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            digits_left = 5;
            cursor = ADDHERO_ENTRY_FIELD_TEXT(g_addhero_card_slot, entry_index);
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
            suffix = (u8*)&g_addhero_entries[g_addhero_card_slot][entry_index].name[ADDHERO_SAVE_FILENAME_PREFIX_LENGTH];
            g_addhero_entry_fields[g_addhero_card_slot][entry_index] = value;
            suffix_value = addhero_parse_hex_suffix_byte(suffix);
            g_addhero_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            g_addhero_entry_fields[g_addhero_card_slot][entry_index] = -1;
            g_addhero_entry_suffix_values[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}

/**
 * @brief Rank the current card's entries by parsed field value, tag "full"
 *        entries, and pick the highest-valued entry to select.
 * @return Index of the highest-valued entry.
 * @see decomp.me (100%)
 */
s32 addhero_rank_entries(void)
{
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;

    addhero_parse_entry_fields();
    maximum = -1;
    addhero_sort_entries_by_type();
    max_suffix = addhero_parse_entry_fields();
    addhero_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (g_addhero_entry_fields[g_addhero_card_slot][entry_index] >= 0)
        {
            if (g_addhero_entry_fields[g_addhero_card_slot][entry_index] >= maximum)
            {
                g_addhero_entry_ranks[entry_index] = next_rank;
                maximum = g_addhero_entry_fields[g_addhero_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_addhero_entry_fields[g_addhero_card_slot][entry_index] <
                        g_addhero_entry_fields[g_addhero_card_slot][previous_index])
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
    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (next_rank < g_addhero_entry_fields[g_addhero_card_slot][entry_index])
        {
            next_rank = g_addhero_entry_fields[g_addhero_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    g_addhero_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            g_addhero_entry_suffix_values[entry_index] = max_suffix + 1;
            break;
        }
    }
    return maximum;
}

/**
 * @brief Reset the 15 per-entry rank slots to -1 and the rank count to 0x28.
 * @see decomp.me (100%)
 */
void addhero_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_addhero_rank_count = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        g_addhero_entry_ranks[i] = val;
    }
}

/**
 * @brief Test whether the active card holds at least one entry matching a known
 *        save-name prefix.
 * @return 1 if a known-type entry exists, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0 ||
            strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sum the block usage of the active card's entries and test whether it
 *        has reached the card's capacity.
 * @return 1 when total used blocks are >= 0xE, 0 otherwise.
 * @note Inlined into addhero_scan_next_entry.
 * @see decomp.me (100%)
 */
inline s32 addhero_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        used_blocks += g_addhero_entries[g_addhero_card_slot][entry_index].size / ADDHERO_CARD_BLOCK_BYTES;
    }
    return used_blocks >= ADDHERO_USED_BLOCK_LIMIT;
}

/**
 * @brief Remove both placeholder save filenames from the active card.
 * @note Inlined into addhero_advance_load_sequence.
 * @see decomp.me (100%)
 */
inline void addhero_erase_placeholder_files(void)
{
    AddheroProbeFilePath buf;

    memcpy(&buf, &g_addhero_file_template, 6);
    buf.device.characters.slot += (u8)g_addhero_card_slot;
    strcat(&buf, g_lom_save_dummy_filename);
    erase(&buf);

    memcpy(&buf, &g_addhero_file_template, 6);
    buf.device.characters.slot += (u8)g_addhero_card_slot;
    strcat(&buf, g_lom_alt_save_dummy_filename);
    erase(&buf);
}

/**
 * @brief Advance the active memory-card load/save sequence by one step.
 * @return One of the ADDHERO_LOAD_RESULT_* values describing how the caller
 *         should continue the sequence.
 * @note g_addhero_load_step walks one of the g_addhero_loadseq_* byte tables;
 *       opcodes with no case are no-ops.
 */
s32 addhero_advance_load_sequence(void)
{
    AddheroSequenceFilePath card_path;
    s32 card_status0;
    s32 card_status1;
    s32 result;
    s32 attempts;
    s32 poll_status;
    s32 entry_index;
    s32 empty_rank;

    memcpy(&card_path, &g_addhero_file_template, 6);
    result = ADDHERO_LOAD_RESULT_PENDING;
    card_path.device.characters.slot += (u8)g_addhero_card_slot;

    if (g_addhero_load_step != NULL)
    {
        switch (*g_addhero_load_step)
        {
        case ADDHERO_STEP_CARD_INFO:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_addhero_card_slot);
            _card_info(g_addhero_card_slot * 0x10);
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_CARD_INFO:
            switch (addhero_poll_software_card_events())
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                g_addhero_load_step++;
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
                result = ADDHERO_LOAD_RESULT_COMPLETE;
                g_addhero_selection_status = 0;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
                g_addhero_load_step++;
                break;
            case ADDHERO_CARD_EVENT_NEW_CARD:
                g_addhero_rank_count = 0x28;
                empty_rank = -1;
                for (entry_index = 14; entry_index >= 0; entry_index--)
                {
                    g_addhero_entry_ranks[entry_index] = empty_rank;
                }
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
                g_addhero_load_step = &g_addhero_loadseq_start;
                break;
            }
            break;

        case ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS:
            addhero_clear_software_card_events();
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_HARDWARE_EVENTS:
            do
            {
                poll_status = addhero_poll_hardware_card_events();
            } while (poll_status == ADDHERO_CARD_EVENT_NONE);
            switch (poll_status)
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                g_addhero_load_step++;
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
            case ADDHERO_CARD_EVENT_NEW_CARD:
                result = ADDHERO_LOAD_RESULT_COMPLETE;
                g_addhero_selection_status = 0;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
                break;
            }
            break;

        case ADDHERO_STEP_CLEAR_HARDWARE_EVENTS:
            addhero_clear_hardware_card_events();
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_SCAN_ENTRIES:
            addhero_erase_placeholder_files();
            g_addhero_entry_scan_active = 1;
            if (addhero_begin_entry_scan(g_addhero_card_slot) == 0)
            {
                result = ADDHERO_LOAD_RESULT_ABORT;
                g_addhero_load_step = NULL;
                g_addhero_entry_state = 0xF8;
                g_addhero_entry_scan_active = 0;
                break;
            }
            g_addhero_load_step++;
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (addhero_scan_next_entry(g_addhero_card_slot) == 0)
                {
                    if (g_addhero_mode != 0)
                    {
                        g_addhero_selected_row = 0;
                    }
                    g_addhero_entry_scan_active = 0;
                    if (g_addhero_entry_state != 0xF8 && g_addhero_entry_state != ADDHERO_ENTRY_STATE_CARD_FULL)
                    {
                        addhero_commit_selected_entry();
                    }
                    break;
                }
            }
            break;

        case ADDHERO_STEP_CLEAR_CARD:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_addhero_card_slot);
            _card_clear(g_addhero_card_slot * 0x10);
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_LOAD_CARD:
            result = ADDHERO_LOAD_RESULT_CONTINUE;
            _card_wait(g_addhero_card_slot);
            _card_load(g_addhero_card_slot * 0x10);
            g_addhero_primary_poll_countdown = 0x10;
            g_addhero_secondary_poll_countdown = 0x10;
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_DONE:
            result = ADDHERO_LOAD_RESULT_ABORT;
            g_addhero_write_in_progress = 0;
            break;

        case ADDHERO_STEP_ERASE_ENTRY:
            strcat(&card_path, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name);
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (erase(&card_path) != 0)
                {
                    break;
                }
            }
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_CARD_LOAD:
            switch (addhero_poll_software_card_events())
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                g_addhero_load_step++;
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
                g_addhero_secondary_poll_countdown--;
                if (g_addhero_secondary_poll_countdown != 0)
                {
                    _card_wait(g_addhero_card_slot);
                    _card_clear(g_addhero_card_slot * 0x10);
                    _card_wait(g_addhero_card_slot);
                    _card_load(g_addhero_card_slot * 0x10);
                }
                else
                {
                    result = ADDHERO_LOAD_RESULT_COMPLETE;
                    g_addhero_selection_status = 0;
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
                }
                break;
            case ADDHERO_CARD_EVENT_NEW_CARD:
                g_addhero_primary_poll_countdown--;
                if (g_addhero_primary_poll_countdown != 0)
                {
                    _card_wait(g_addhero_card_slot);
                    _card_clear(g_addhero_card_slot * 0x10);
                    _card_wait(g_addhero_card_slot);
                    _card_load(g_addhero_card_slot * 0x10);
                }
                else
                {
                    result = ADDHERO_LOAD_RESULT_CARD_ERROR;
                    g_addhero_entry_state = 0xFC;
                    g_addhero_load_step = g_addhero_loadseq_card;
                }
                break;
            }
            break;

        case ADDHERO_STEP_WAIT_HARDWARE_EVENTS:
            do
            {
                poll_status = addhero_poll_hardware_card_events();
            } while (poll_status == ADDHERO_CARD_EVENT_NONE);
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_READ_ENTRY:
            g_addhero_io_busy = 1;
            g_addhero_selection_status = 0;
            _card_wait(g_addhero_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            if (g_addhero_file_handle == -1)
            {
                break;
            }
            addhero_clear_software_card_events();
            _card_wait(g_addhero_card_slot);
            if (read(g_addhero_file_handle, &g_addhero_entry_read_buffer, g_addhero_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
            {
                close(g_addhero_file_handle);
                break;
            }
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_ENTRY_READ:
            if (g_addhero_io_busy != 0)
            {
                poll_status = addhero_poll_software_card_events();
                if (poll_status == ADDHERO_CARD_EVENT_COMPLETE)
                {
                    g_addhero_io_busy = 0;
                    g_addhero_selection_status = 1;
                    close(g_addhero_file_handle);
                }
                else if (poll_status != ADDHERO_CARD_EVENT_NONE)
                {
                    close(g_addhero_file_handle);
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
                    g_addhero_load_step = &g_addhero_loadseq_start;
                }
            }
            else
            {
                g_addhero_load_step++;
            }
            break;

        case ADDHERO_STEP_READ_SAVE:
            g_addhero_progress_active = 1;
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_addhero_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            addhero_clear_software_card_events();
            _card_wait(g_addhero_card_slot);
            if (read(g_addhero_file_handle, g_addhero_save_blob, ADDHERO_SAVE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_status_dialog(1);
                    return result;
                }
                break;
            }
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_SAVE_READ:
            switch (addhero_poll_software_card_events())
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                g_addhero_progress_active = 0;
                g_addhero_load_step++;
                close(g_addhero_file_handle);
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
            case ADDHERO_CARD_EVENT_NEW_CARD:
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_addhero_progress_bar_active = 0;
                    addhero_open_status_dialog(1);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_READ_SAVE and try again. */
                g_addhero_load_step--;
                break;
            }
            break;

        case ADDHERO_STEP_CHECK_CARD_TYPE:
            for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
            {
                if (McxCardType(g_addhero_card_slot * 0x10) == 1)
                {
                    break;
                }
                VSync(0);
            }
            if (attempts != ADDHERO_FILE_OP_ATTEMPTS)
            {
                func_80032174(0, &card_status0, &card_status1);
                if (card_status1 == 0)
                {
                    g_addhero_load_step++;
                    break;
                }
            }
            addhero_open_status_dialog(3);
            break;

        case ADDHERO_STEP_INIT_RETRIES:
            g_addhero_retry_count = ADDHERO_SAVE_RETRIES;
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_READ_BEFORE_WRITE:
            g_addhero_progress_active = 1;
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_addhero_card_slot);
            g_addhero_file_handle = open(g_addhero_save_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_READ);
            addhero_clear_software_card_events();
            _card_wait(g_addhero_card_slot);
            if (read(g_addhero_file_handle, g_addhero_save_blob, ADDHERO_SAVE_BYTES) == -1)
            {
                close(g_addhero_file_handle);
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(1);
                    return result;
                }
                break;
            }
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_PREWRITE_READ:
            switch (addhero_poll_software_card_events())
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                g_addhero_progress_active = 0;
                g_addhero_load_step++;
                close(g_addhero_file_handle);
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
            case ADDHERO_CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_addhero_progress_bar_active = 0;
                    addhero_open_exit_dialog(1);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_READ_BEFORE_WRITE and try again. */
                close(g_addhero_file_handle);
                g_addhero_load_step--;
                break;
            }
            break;

        case ADDHERO_STEP_WRITE_SAVE:
            if (g_addhero_has_free_entry_space == 0)
            {
                _card_wait(g_addhero_card_slot);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(g_addhero_save_file_path) != 0)
                    {
                        break;
                    }
                }
            }
            strcat(&card_path, g_lom_save_dummy_filename);
            _card_wait(g_addhero_card_slot);
            g_addhero_file_handle = open(&card_path, ADDHERO_FILE_CREATE | ADDHERO_FILE_BLOCKS(2));
            if (g_addhero_file_handle == -1)
            {
                close(-1);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (erase(&card_path) != 0)
                    {
                        break;
                    }
                }
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    addhero_open_exit_dialog(0);
                    return result;
                }
                break;
            }
            close(g_addhero_file_handle);
            strcpy(g_addhero_target_file_path, &card_path);
            _card_wait(g_addhero_card_slot);
            g_addhero_file_handle = open(g_addhero_target_file_path, ADDHERO_FILE_ASYNC | ADDHERO_FILE_WRITE);
            addhero_clear_software_card_events();
            g_addhero_progress_start_tick = VSync(-1);
            g_addhero_progress_bar_active = 1;
            _card_wait(g_addhero_card_slot);
            if (write(g_addhero_file_handle, g_addhero_save_blob, ADDHERO_SAVE_BYTES) == -1)
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
                    addhero_open_exit_dialog(0);
                    return result;
                }
                break;
            }
            g_addhero_load_step++;
            break;

        case ADDHERO_STEP_POLL_SAVE_WRITE:
            switch (addhero_poll_software_card_events())
            {
            case ADDHERO_CARD_EVENT_COMPLETE:
                if (g_addhero_has_free_entry_space != 0)
                {
                    _card_wait(g_addhero_card_slot);
                    for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                    {
                        if (erase(g_addhero_save_file_path) != 0)
                        {
                            break;
                        }
                    }
                }
                _card_wait(g_addhero_card_slot);
                for (attempts = 0; attempts < ADDHERO_FILE_OP_ATTEMPTS; attempts++)
                {
                    if (rename(g_addhero_target_file_path, g_addhero_save_file_path) != 0)
                    {
                        break;
                    }
                }
                g_addhero_write_in_progress = 0;
                g_addhero_load_step++;
                close(g_addhero_file_handle);
                break;
            case ADDHERO_CARD_EVENT_ERROR:
            case ADDHERO_CARD_EVENT_TIMEOUT:
            case ADDHERO_CARD_EVENT_NEW_CARD:
                g_addhero_retry_count--;
                if (g_addhero_retry_count == 0)
                {
                    g_addhero_progress_bar_active = 0;
                    addhero_open_exit_dialog(0);
                    return result;
                }
                /* Rewind to ADDHERO_STEP_WRITE_SAVE and try again. */
                close(g_addhero_file_handle);
                g_addhero_load_step--;
                break;
            }
            break;
        }
    }
    return result;
}

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
    if (event_status != ADDHERO_CARD_EVENT_NONE)
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
    s32 selected_entry;

    if (nextfile(&g_addhero_entries[page][g_addhero_entry_state]) != 0)
    {
        func_800B0170(&g_addhero_entries[page][g_addhero_entry_state]);
        g_addhero_entry_state += 1;
        return 1;
    }

    field_reset_input_repeat();
    if ((g_addhero_mode == 0) && (addhero_has_known_entry_type() == 0))
    {
        g_addhero_entry_state = 0xF8;
    }
    else
    {
        g_addhero_has_free_entry_space = 0;
        if (addhero_entry_blocks_reach_limit())
        {
            selected_entry = addhero_rank_entries();
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_FULL;
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
 * @see decomp.me (100.00%)
 */
void addhero_commit_selected_entry(void)
{
    AddheroSelectedFilePath path;

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
    strcat(&path, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name);
    path.device.characters.slot += (u8)g_addhero_card_slot;
    g_addhero_selection_status = 0;
    strcpy(g_addhero_save_file_path, &path);
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
 * @return An ADDHERO_CARD_EVENT_* result; NONE when no event is pending.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_software_card_events(void)
{
    if (TestEvent(g_addhero_software_event_io_complete) == 1)
    {
        return ADDHERO_CARD_EVENT_COMPLETE;
    }
    if (TestEvent(g_addhero_software_event_error) == 1)
    {
        return ADDHERO_CARD_EVENT_ERROR;
    }
    if (TestEvent(g_addhero_software_event_timeout) == 1)
    {
        return ADDHERO_CARD_EVENT_TIMEOUT;
    }
    if (TestEvent(g_addhero_software_event_new_card) == 1)
    {
        return ADDHERO_CARD_EVENT_NEW_CARD;
    }
    return ADDHERO_CARD_EVENT_NONE;
}

/**
 * @brief Consume the first pending hardware memory-card event.
 * @return An ADDHERO_CARD_EVENT_* result; NONE when no event is pending.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_hardware_card_events(void)
{
    if (TestEvent(g_addhero_hardware_event_io_complete) == 1)
    {
        return ADDHERO_CARD_EVENT_COMPLETE;
    }
    if (TestEvent(g_addhero_hardware_event_error) == 1)
    {
        return ADDHERO_CARD_EVENT_ERROR;
    }
    if (TestEvent(g_addhero_hardware_event_timeout) == 1)
    {
        return ADDHERO_CARD_EVENT_TIMEOUT;
    }
    if (TestEvent(g_addhero_hardware_event_new_card) == 1)
    {
        return ADDHERO_CARD_EVENT_NEW_CARD;
    }
    return ADDHERO_CARD_EVENT_NONE;
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
