#include "common/card_directory.h"
#include "common/card_events.h"
#include "common/save_file.h"

/**
 * @brief Read save-file serials and suffixes from the current card's directory.
 *
 * Save filenames contain a hex serial, a separator, then a two-digit hex suffix.
 * Store up to CARD_SERIAL_DIGITS serial digits in g_card_entry_fields and the
 * suffix in g_card_entry_suffix_values. Other files receive CARD_ENTRY_SERIAL_NONE
 * and zero, respectively.
 *
 * @return Largest save-file suffix, or zero if no save files are present.
 * @see decomp.me (100%, ADDHERO copy) https://decomp.me/scratch/7hY8R
 */
s32 parse_entry_fields(void)
{
    s32 entry_index;
    s32 largest_suffix;
    u8* serial_text;
    const char* filename_fields;
    s32 digits_left;
    s32 serial_value;
    s32 suffix_value;

    largest_suffix = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            digits_left = CARD_SERIAL_DIGITS;
            serial_text = CARD_ENTRY_SERIAL_TEXT(g_card_slot, entry_index);
            serial_value = 0;
            while ((*serial_text >= '0' && *serial_text <= '9') ||
                   (*serial_text >= 'a' && *serial_text <= 'f') ||
                   (*serial_text >= 'A' && *serial_text <= 'F'))
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

            /* The suffix follows the full serial, even when it exceeds the digit limit. */
            filename_fields = &g_card_entries[g_card_slot][entry_index].name[CARD_SAVE_FILENAME_PREFIX_LENGTH];
            g_card_entry_fields[g_card_slot][entry_index] = serial_value;
            suffix_value = parse_hex_suffix_byte(filename_fields);
            g_card_entry_suffix_values[entry_index] = suffix_value;
            if (largest_suffix < suffix_value)
            {
                largest_suffix = suffix_value;
            }
        }
        else
        {
            g_card_entry_fields[g_card_slot][entry_index] = CARD_ENTRY_SERIAL_NONE;
            g_card_entry_suffix_values[entry_index] = 0;
        }
    }
    return largest_suffix;
}
