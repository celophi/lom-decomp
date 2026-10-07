/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Zero-fill a card title from its first null byte at a character boundary.
 * @param title_text Both lines of SaveFileHeader::title, scanned as one buffer.
 * @note Bytes at or above SJIS_LEAD_MIN skip two bytes without validating the character.
 *       A title with no terminator at a character boundary is left unchanged.
 */
void terminate_multibyte_text(void* title_text)
{
    u8* cursor;
    s32 byte_offset;

    cursor = (u8*)title_text;
    byte_offset = 0;
    while (1)
    {
        if (byte_offset >= SAVE_FILE_TITLE_BYTES)
        {
            return;
        }
        if (*cursor == 0)
        {
            for (; byte_offset < SAVE_FILE_TITLE_BYTES; byte_offset++, cursor++)
            {
                *cursor = 0;
            }
            return;
        }
        if (*cursor >= SJIS_LEAD_MIN)
        {
            cursor += SJIS_MULTIBYTE_CHAR_BYTES;
            byte_offset += SJIS_MULTIBYTE_CHAR_BYTES;
        }
        else
        {
            cursor++;
            byte_offset++;
        }
    }
}
