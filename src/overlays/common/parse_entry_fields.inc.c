/*
 * Shared memory-card directory function; see include/card_directory.h. Included by
 * each overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "card_directory.h"
#include "card_events.h"
#include "save_file.h"

/**
 * @brief Parse the hex serial and suffix byte of every Legend of Mana file on the current card.
 *
 * Entries named with g_lom_save_filename_prefix have up to CARD_SERIAL_DIGITS hex
 * digits after it parsed into g_card_entry_fields, and the byte after that run
 * into g_card_entry_suffix_values; other entries store -1 and 0.
 *
 * @return Largest suffix byte among the Legend of Mana entries, or 0 when there are none.
 * @see decomp.me (100%, ADDHERO copy) https://decomp.me/scratch/7hY8R
 */
s32 parse_entry_fields(void)
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
            suffix_value = parse_hex_suffix_byte(suffix);
            g_card_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            g_card_entry_fields[g_card_slot][entry_index] = -1;
            g_card_entry_suffix_values[entry_index] = 0;
        }
    }
    return max_suffix;
}
