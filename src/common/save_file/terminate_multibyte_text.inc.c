/*
 * Shared save-file function; see include/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "save_file.h"

/**
 * @brief Zero-fill a card header title from its first null byte onward,
 *        walking Shift-JIS characters two bytes at a time.
 * @param text Start of the title (SaveFileHeader::title, two lines).
 */
void terminate_multibyte_text(void* text)
{
    u8* p;
    s32 i;

    p = (u8*)text;
    i = 0;
    while (1)
    {
        if (i >= SAVE_FILE_TITLE_LINE_BYTES * 2)
        {
            return;
        }
        if (*p == 0)
        {
            for (; i < SAVE_FILE_TITLE_LINE_BYTES * 2; i++, p++)
            {
                *p = 0;
            }
            return;
        }
        if (*p >= SJIS_LEAD_MIN)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}
